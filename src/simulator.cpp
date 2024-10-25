#include <memory>
#include <iostream>
#include <fstream>
#include <utility>
#include <string>
#include <map>

#include <gsl/gsl_rng.h>
#include <SQLiteCpp/SQLiteCpp.h>
#include <nlohmann/json.hpp>

#include "simulator.hpp"
#include "community.hpp"
#include "parameters.hpp"
#include "person.hpp"
#include "ledger.hpp"

using json = nlohmann::json;

RngHandler::RngHandler(unsigned long int seed) {
    infection_rng   = gsl_rng_alloc(gsl_rng_mt19937);
    vaccination_rng = gsl_rng_alloc(gsl_rng_mt19937);
    behavior_rng    = gsl_rng_alloc(gsl_rng_mt19937);

    gsl_rng_set(infection_rng, seed);
    gsl_rng_set(vaccination_rng, seed);
    gsl_rng_set(behavior_rng, seed);
}

RngHandler::~RngHandler() {
    gsl_rng_free(infection_rng);
    gsl_rng_free(vaccination_rng);
    gsl_rng_free(behavior_rng);
}

double RngHandler::draw_from_rng(RngType type) const {
    switch (type) {
        case INFECTION:   { return gsl_rng_uniform(infection_rng); }
        case VACCINATION: { return gsl_rng_uniform(vaccination_rng); }
        case BEHAVIOR:    { return gsl_rng_uniform(behavior_rng); }
        default:          { return gsl_rng_uniform(infection_rng); }
    }
}

gsl_rng* RngHandler::get_rng(RngType type) const {
    switch (type) {
        case INFECTION:   { return infection_rng; }
        case VACCINATION: { return vaccination_rng; }
        case BEHAVIOR:    { return behavior_rng; }
        default:          { return infection_rng; }
    }
}

Simulator::Simulator() : sim_time(0) {
    rng_seed = 0;
    rng_handler = std::make_unique<RngHandler>(rng_seed);
    par = std::make_unique<Parameters>(rng_handler.get());
    community = std::make_unique<Community>(par.get(), rng_handler.get());
}

Simulator::Simulator(std::string config, size_t serial) : sim_time(0) {
    std::map<std::string, double> model_params;

    std::ifstream cfg_file(config);
    auto cfg = json::parse(cfg_file);
    for (auto& [key, el] : cfg["model_parameters"].items()) {
        model_params[el["fullname"]] = 0.0;
    }
    cfg_file.close();

    std::string db_path = cfg["experiment_name"];
    db_path += std::string(".sqlite");
    SQLite::Database db(db_path);
    SQLite::Statement query(db, "SELECT * FROM par WHERE serial = ?");
    query.bind(1, (unsigned int) serial);
    {
        while (query.executeStep()) {
            model_params["seed"] = query.getColumn("seed");
            for (auto& [param, val] : model_params) {
                model_params[param] = query.getColumn(param.c_str());
            }
        }
    }

    rng_handler = std::make_unique<RngHandler>(model_params["seed"]);
    par = std::make_unique<Parameters>(rng_handler.get(), model_params);
    community = std::make_unique<Community>(par.get(), rng_handler.get());
    
}

Simulator::~Simulator() {}

void Simulator::init() {
    community->vaccinate_population(sim_time);
}

void Simulator::simulate() {
    for (; sim_time < par->simulation_duration; ++sim_time) {
        tick();
    }
}

void Simulator::tick() {
    // par->update_time_varying_parameters();
    community->transmission(sim_time);
}

void Simulator::results() {
    auto ledger = community->ledger.get();

    // { // generate linelist csv (sim.linelist)
    //     ledger->generate_linelist_csv();
    // }

    { // generate simvis csv (sim.vis)
        ledger->calculate_cumulatives();
        ledger->calculate_tnd_ve_est();
        ledger->generate_simvis_csv();
    }

    auto total_vaxd_flu_infs = ledger->total_infections(VACCINATED, INFLUENZA);
    auto total_vaxd_flu_cases = ledger->total_sympt_infections(VACCINATED, INFLUENZA);
    auto total_vaxd_flu_mai = ledger->total_mai(VACCINATED, INFLUENZA);
    auto total_vaxd_nonflu_infs = ledger->total_infections(VACCINATED, NON_INFLUENZA);
    auto total_vaxd_nonflu_cases = ledger->total_sympt_infections(VACCINATED, NON_INFLUENZA);
    auto total_vaxd_nonflu_mai = ledger->total_mai(VACCINATED, NON_INFLUENZA);
    auto vax_coverage = (double) ledger->total_vaccinations() / par->population_size;
    auto final_tnd_ve = ledger->get_tnd_ve_est(par->simulation_duration - 1);

    std::cerr << "rng seed:            " << rng_seed << '\n'
              << "flu infs (cAR%):     " << total_vaxd_flu_infs << " (" << ((double) total_vaxd_flu_infs/par->population_size)*100 << "%)" << '\n'
              << "flu cases (inf%):    " << total_vaxd_flu_cases << " (" << ((double) total_vaxd_flu_cases/total_vaxd_flu_infs)*100 << "%)" << '\n'
              << "flu mais (inf%):     " << total_vaxd_flu_mai << " (" << ((double) total_vaxd_flu_mai/total_vaxd_flu_infs)*100 << "%)" << '\n'
              << "nonflu infs (cAR%):  " << total_vaxd_nonflu_infs << " (" << ((double) total_vaxd_nonflu_infs/par->population_size)*100 << "%)" << '\n'
              << "nonflu cases (inf%): " << total_vaxd_nonflu_cases << " (" << ((double) total_vaxd_nonflu_cases/total_vaxd_nonflu_infs)*100 << "%)" << '\n'
              << "nonflu mais (inf%):  " << total_vaxd_nonflu_mai << " (" << ((double) total_vaxd_nonflu_mai/total_vaxd_nonflu_infs)*100 << "%)" << '\n'
              << "final tnd ve (vax%): " << final_tnd_ve << " ("<< vax_coverage*100 << "%)" << '\n';
}