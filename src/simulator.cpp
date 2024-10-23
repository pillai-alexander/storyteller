#include <memory>
#include <iostream>

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

void Simulator::results() {
    auto ledger = community->ledger.get();
    auto total_flu_infs = ledger->total_infections(INFLUENZA);
    auto total_flu_cases = ledger->total_sympt_infections(INFLUENZA);
    auto total_flu_mai = ledger->total_mai(INFLUENZA);
    auto total_nonflu_infs = ledger->total_infections(NON_INFLUENZA);
    auto total_nonflu_cases = ledger->total_sympt_infections(NON_INFLUENZA);
    auto total_nonflu_mai = ledger->total_mai(NON_INFLUENZA);
    auto vax_coverage = (double) ledger->total_vaccinations() / par->population_size;

    std::cerr << "rng seed:         " << rng_seed << '\n'
              << "flu infs:         " << total_flu_infs << '\n'
              << "flu cases (%):    " << total_flu_cases << " (" << (total_flu_cases/total_flu_infs)*100 << "%)" << '\n'
              << "flu mais (%):     " << total_flu_mai << " (" << (total_flu_mai/total_flu_infs)*100 << "%)" << '\n'
              << "nonflu infs:      " << total_nonflu_infs << '\n'
              << "nonflu cases (%): " << total_nonflu_cases << " (" << (total_nonflu_cases/total_nonflu_infs)*100 << "%)" << '\n'
              << "nonflu mais (%):  " << total_nonflu_mai << " (" << (total_nonflu_mai/total_nonflu_infs)*100 << "%)" << '\n'
              << "vax coverage:     " << vax_coverage*100 << "%" << '\n';

    int n_ppl = 0;
    std::cout << "person_id,susceptibility,vax_protection,vax_status,inf_time,inf_strain,inf_symptoms,inf_care" << '\n';
    for (auto& p : community->people) {
        auto suscep = p->get_susceptibility();
        auto vax_prot = p->get_vaccine_protection(INFLUENZA);
        auto vax_status = p->is_vaccinated();
        auto inf = p->has_been_infected() ? p->most_recent_infection() : nullptr;
        int inf_time = inf ? inf->get_infection_time() : -1;
        auto inf_strain = inf ? inf->get_strain() : NUM_STRAIN_TYPES;
        auto inf_sympt = inf ? inf->get_symptoms() : NUM_SYMPTOM_CLASSES;
        auto inf_care = inf ? inf->get_sought_care() : false;
        std::cout
            << ++n_ppl << ','
            << suscep << ','
            << vax_prot << ','
            << vax_status << ','
            << inf_time << ','
            << inf_strain << ','
            << inf_sympt << ','
            << inf_care << '\n';
    }
}