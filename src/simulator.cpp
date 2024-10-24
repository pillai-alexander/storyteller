#include <memory>
#include <iostream>
#include <fstream>
#include <utility>
#include <string>

#include <gsl/gsl_rng.h>

#include "simulator.hpp"
#include "community.hpp"
#include "parameters.hpp"
#include "person.hpp"

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

LineList::LineList(const Parameters* parameters) {
    par = parameters;
    header = "inf_id,inf_time,inf_strain,inf_sympts,inf_care,p_id,vax_status,baseline_suscep,vax_effect";
}

LineList::~LineList() {}

void LineList::log_infection(const Infection* i) {
    infections.push_back(i);
}

void LineList::generate_linelist_csv(std::string filepath) {
    extract_infection_information();

    if (filepath.empty()) filepath = par->linelist_file_path;
    std::ofstream file(filepath);
    file << header << '\n';
    for (size_t i = 0; i < infections.size(); ++i) {
        file << i << ','
             << inf_time[i] << ','
             << inf_strain[i] << ','
             << inf_symptoms[i] << ','
             << inf_care[i] << ','
             << person_id[i] << ','
             << vax_status[i] << ','
             << baseline_suscep[i] << ','
             << vax_effect[i] << '\n';
    }
    file.close();
}

void LineList::extract_infection_information() {
    auto n_infs = infections.size();
    inf_time = std::vector<size_t>(n_infs);
    inf_strain = std::vector<StrainType>(n_infs);
    inf_symptoms = std::vector<SymptomClass>(n_infs);
    inf_care = std::vector<bool>(n_infs);
    person_id = std::vector<size_t>(n_infs);
    vax_status = std::vector<VaccinationStatus>(n_infs);
    baseline_suscep = std::vector<double>(n_infs);
    vax_effect = std::vector<double>(n_infs);

    for (size_t i = 0; i < n_infs; ++i) {
        auto infection = infections[i];
        auto strain = infection->get_strain();
        auto infectee = infection->get_infectee();

        inf_time[i] = infection->get_infection_time();
        inf_strain[i] = strain;
        inf_symptoms[i] = infection->get_symptoms();
        inf_care[i] = infection->get_sought_care();
        person_id[i] = infectee->get_id();
        vax_status[i] = (VaccinationStatus) infectee->is_vaccinated();
        baseline_suscep[i] = infectee->get_susceptibility(strain);
        vax_effect[i] = infectee->get_vaccine_protection(strain);
    }
}

Simulator::Simulator() {
    sim_time = 0;
    rng_seed = 0;
    rng_handler = std::make_unique<RngHandler>(rng_seed);
    par = std::make_unique<Parameters>(rng_handler.get());
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

LineList Simulator::results() {
    auto ledger = community->ledger.get();
    auto total_vaxd_flu_infs = ledger->total_infections(VACCINATED, INFLUENZA);
    auto total_vaxd_flu_cases = ledger->total_sympt_infections(VACCINATED, INFLUENZA);
    auto total_vaxd_flu_mai = ledger->total_mai(VACCINATED, INFLUENZA);
    auto total_vaxd_nonflu_infs = ledger->total_infections(VACCINATED, NON_INFLUENZA);
    auto total_vaxd_nonflu_cases = ledger->total_sympt_infections(VACCINATED, NON_INFLUENZA);
    auto total_vaxd_nonflu_mai = ledger->total_mai(VACCINATED, NON_INFLUENZA);
    auto vax_coverage = (double) ledger->total_vaccinations() / par->population_size;

    std::cerr << "rng seed:            " << rng_seed << '\n'
              << "flu infs (cAR%):     " << total_vaxd_flu_infs << " (" << ((double) total_vaxd_flu_infs/par->population_size)*100 << "%)" << '\n'
              << "flu cases (inf%):    " << total_vaxd_flu_cases << " (" << ((double) total_vaxd_flu_cases/total_vaxd_flu_infs)*100 << "%)" << '\n'
              << "flu mais (inf%):     " << total_vaxd_flu_mai << " (" << ((double) total_vaxd_flu_mai/total_vaxd_flu_infs)*100 << "%)" << '\n'
              << "nonflu infs (cAR%):  " << total_vaxd_nonflu_infs << " (" << ((double) total_vaxd_nonflu_infs/par->population_size)*100 << "%)" << '\n'
              << "nonflu cases (inf%): " << total_vaxd_nonflu_cases << " (" << ((double) total_vaxd_nonflu_cases/total_vaxd_nonflu_infs)*100 << "%)" << '\n'
              << "nonflu mais (inf%):  " << total_vaxd_nonflu_mai << " (" << ((double) total_vaxd_nonflu_mai/total_vaxd_nonflu_infs)*100 << "%)" << '\n'
              << "vax coverage (%):    " << vax_coverage*100 << "%" << '\n';

    LineList ll(par.get());
    for (auto& p : community->people) {
        for (auto& i : p->get_infection_history()) {
            ll.log_infection(i.get());
        }
    }

    return std::move(ll);
}