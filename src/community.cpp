#include <algorithm>
#include <memory>

#include <gsl/gsl_randist.h>

#include "community.hpp"
#include "parameters.hpp"
#include "person.hpp"
#include "simulator.hpp"
#include "utility.hpp"
#include "ledger.hpp"

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
    for (auto& p : people) {
        // determine if exposure with occurs
        auto strain = par->sample_strain();
        if (strain == NUM_STRAIN_TYPES) continue;
        // determine if infection occurs
        auto infection_occurs = p->attempt_infection(strain, time);
        if (infection_occurs) ledger->log_infection(infection_occurs);
    }
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