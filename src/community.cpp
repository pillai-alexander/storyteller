#include <algorithm>
#include <memory>
#include <numeric>

#include <gsl/gsl_randist.h>

#include "community.hpp"
#include "parameters.hpp"
#include "person.hpp"
#include "simulator.hpp"

Ledger::Ledger(const Parameters* parameters) {
    par = parameters;
    inf_incidence       = std::vector<std::vector<size_t>>(NUM_STRAIN_TYPES, std::vector<size_t>(par->simulation_duration, 0));
    sympt_inf_incidence = std::vector<std::vector<size_t>>(NUM_STRAIN_TYPES, std::vector<size_t>(par->simulation_duration, 0));
    mai_incidence       = std::vector<std::vector<size_t>>(NUM_STRAIN_TYPES, std::vector<size_t>(par->simulation_duration, 0));

    vax_incidence = std::vector<size_t>(par->simulation_duration, 0);
}

Ledger::~Ledger() {}

std::vector<std::vector<size_t>> Ledger::get_inf_incidence() const { return inf_incidence; }
std::vector<std::vector<size_t>> Ledger::get_sympt_inf_incidence() const { return sympt_inf_incidence; }
std::vector<std::vector<size_t>> Ledger::get_mai_incidence() const { return mai_incidence; }
std::vector<size_t> Ledger::get_vax_incidence() const { return vax_incidence; }

size_t Ledger::total_infections(StrainType strain) const {
    return std::accumulate(inf_incidence[strain].begin(),
                                 inf_incidence[strain].end(),
                                 0);
}

size_t Ledger::total_sympt_infections(StrainType strain) const {
    return std::accumulate(sympt_inf_incidence[strain].begin(),
                           sympt_inf_incidence[strain].end(),
                           0);
}

size_t Ledger::total_mai(StrainType strain) const {
    return std::accumulate(mai_incidence[strain].begin(),
                           mai_incidence[strain].end(),
                           0);
}

size_t Ledger::total_vaccinations() const {
    return std::accumulate(vax_incidence.begin(),
                           vax_incidence.end(),
                           0);
}

Community::Community(const Parameters* parameters, const RngHandler* rng_handler) {
    par = parameters;
    rng = rng_handler;

    ledger = std::make_unique<Ledger>(par);
    
    people.reserve(par->population_size);
    init_population();
}

Community::~Community() {}

void Community::init_population() {
    for (size_t i = 0; i < par->population_size; ++i) {
        people.push_back(std::make_unique<Person>(i, par, rng));
        Person* p = people.back().get();

        susceptibles.push_back(p);
    }
}

void Community::transmission(size_t time) {
    // std::vector<Person*> tomorrow_susceptibles;
    // tomorrow_susceptibles.reserve(susceptibles.size());
    for (auto& p : people) {
        // determine if exposure with occurs
        auto strain = par->sample_strain();
        if (strain == NUM_STRAIN_TYPES) {
            // tomorrow_susceptibles.push_back(p);
            continue;
        }
        // determine if infection occurs
        auto infection_occurs = p->attempt_infection(strain, time);
        if (infection_occurs) {
            auto strain = infection_occurs->get_strain();
            auto sympts = infection_occurs->get_symptoms();
            auto mai    = infection_occurs->get_sought_care();

            ledger->inf_incidence[strain][time]++;
            if (sympts == SYMPTOMATIC) ledger->sympt_inf_incidence[strain][time]++;
            if (mai) ledger->mai_incidence[strain][time]++;

            // if(strain == NON_INFLUENZA) tomorrow_susceptibles.push_back(p);
        }
    }
    // tomorrow_susceptibles.shrink_to_fit();
    // susceptibles = tomorrow_susceptibles;
}

void Community::vaccinate_population(size_t time) {
    if (par->pr_vaccination == 0) { return; }
    for (auto& p : people) {
        if (rng->draw_from_rng(VACCINATION) < par->pr_vaccination) {
            p->vaccinate();
            ledger->vax_incidence[time]++;
        }
    }
}