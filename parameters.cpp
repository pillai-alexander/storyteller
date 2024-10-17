#include <algorithm>
#include <vector>
#include <iostream>

#include <gsl/gsl_randist.h>

#include "parameters.hpp"
#include "person.hpp"
#include "simulator.hpp"

namespace constants {
    unsigned int ONE = 1;
}

Parameters::Parameters(const RngHandler* rng_handler) {
    rng = rng_handler;

    pr_vaccination = 0.5;
    pr_infection = std::vector<double>(NUM_STRAIN_TYPES, 0.01);
    pr_symptoms = std::vector<double>(NUM_STRAIN_TYPES, 0.1);
    pr_seek_care = std::vector<double>(NUM_VACCINATION_STATUSES, 0.25);

    strain_probs = {
        pr_infection[INFLUENZA],
        pr_infection[NON_INFLUENZA],
        1 - (pr_infection[INFLUENZA] + pr_infection[NON_INFLUENZA])
    };
}

Parameters::~Parameters() {}

double Parameters::sample_susceptibility(const Person* p) const {
    if (p->is_vaccinated()) {
        return 0.5;
    } else {
        return 1.0;
    }
}

StrainType Parameters::sample_strain() const {
    const size_t num_categories = NUM_STRAIN_TYPES + 1;
    std::vector<unsigned int> sample(num_categories, 0);
    gsl_ran_multinomial(
        rng->get_rng(INFECTION),
        num_categories,
        constants::ONE,
        strain_probs.data(),
        sample.data()
    );

    auto idx = std::find(sample.begin(), sample.end(), constants::ONE) - sample.begin();
    return (StrainType) idx;
}

void Parameters::update_time_varying_parameters() {}