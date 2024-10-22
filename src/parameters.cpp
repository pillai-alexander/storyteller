#include <algorithm>
#include <vector>

#include <gsl/gsl_randist.h>

#include "parameters.hpp"
#include "person.hpp"
#include "simulator.hpp"

namespace constants {
    unsigned int ONE = 1;
}

Parameters::Parameters(const RngHandler* rng_handler) {
    rng = rng_handler;

    init_parameters();
}

Parameters::~Parameters() {}

void Parameters::init_parameters() {
    population_size = (size_t) 1e3;
    simulation_duration = 200;

    pr_vaccination = 0.5;
    pr_infection = std::vector<double>(NUM_STRAIN_TYPES, 0.01);
    pr_symptoms = std::vector<double>(NUM_STRAIN_TYPES, 1);
    pr_seek_care = std::vector<double>(NUM_VACCINATION_STATUSES, 1);

    baseline_suscep_distr_shape = 1;
    baseline_suscep_distr_mean = {1, 0.5};

    vax_effect_distr_params = {1, 1};

    strain_probs = {
        pr_infection[INFLUENZA],
        pr_infection[NON_INFLUENZA],
        1 - (pr_infection[INFLUENZA] + pr_infection[NON_INFLUENZA])
    };
}

double Parameters::sample_susceptibility(const Person* p) const {
    if (p->is_vaccinated()) {
        return gsl_ran_gamma(
            rng->get_rng(INFECTION),
            baseline_suscep_distr_shape,
            baseline_suscep_distr_mean[VACCINATED] / baseline_suscep_distr_shape
        );
    } else {
        return gsl_ran_gamma(
            rng->get_rng(INFECTION),
            baseline_suscep_distr_shape,
            baseline_suscep_distr_mean[UNVACCINATED] / baseline_suscep_distr_shape
        );
    }
}

double Parameters::sample_vaccine_effect() const {
    return gsl_ran_beta(
        rng->get_rng(VACCINATION),
        vax_effect_distr_params[A],
        vax_effect_distr_params[B]
    );
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