#include <algorithm>

#include <gsl/gsl_randist.h>

#include "community.hpp"
#include "parameters.hpp"
#include "person.hpp"
#include "simulator.hpp"

Community::Community(const Parameters* parameters, const RngHandler* rng_handler) {
    par = parameters;
    rng = rng_handler;

    cumulative_infections = std::vector<size_t>(NUM_STRAIN_TYPES, 0);
    
    people.reserve(par->population_size);
    init_population();
}

Community::~Community() {}

void Community::init_population() {
    for (size_t i = 0; i < par->population_size; ++i) {
        people.push_back(std::make_unique<Person>(par, rng));
        Person* p = people.back().get();

        susceptibles.push_back(p);
    }
}

void Community::transmission(size_t time) {
    std::vector<Person*> tomorrow_susceptibles;
    for (auto p : susceptibles) {
        // determine if exposure with occurs
        auto strain = par->sample_strain();
        if (strain == NUM_STRAIN_TYPES) {
            tomorrow_susceptibles.push_back(p); 
            continue;
        }
        // determine if infection occurs
        auto infection_occurs = p->infect(strain, time);
        if (infection_occurs) {
            cumulative_infections[strain]++;
        }
    }
    susceptibles = tomorrow_susceptibles;
}

void Community::vaccinate_population() {
    if (par->pr_vaccination == 0) { return; }
    for (auto& p : people) {
        if (rng->draw_from_rng(VACCINATION) < par->pr_vaccination) {
            p->vaccinate();
        }
    }
}