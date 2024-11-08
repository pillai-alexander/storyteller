/**
 * @file parameters.cpp
 * @author Alexander N. Pillai
 * @brief Contains the Parameters class that stores all necessary simulaiton
 *        parameters and contains helper functions that use the parameter values.
 *
 * @copyright TBD
 */
#include <algorithm>
#include <vector>
#include <array>
#include <numeric>
#include <string>

#include <gsl/gsl_randist.h>

#include <storyteller/parameters.hpp>
#include <storyteller/person.hpp>
#include <storyteller/simulator.hpp>
#include <storyteller/utility.hpp>

Parameters::Parameters(const RngHandler* rngh) {
    rng = rngh;
    init_parameters();
}

Parameters::Parameters(const RngHandler* rngh, std::map<std::string, double> cfg_params) : Parameters(rngh) {
    simulation_duration = cfg_params["sim_duration"];
    database_path       = cfg_params["db_path"];
    population_size     = cfg_params["pop_size"];

    suscep_distr_params[VACCINATED][INFLUENZA][SHAPE]     = cfg_params["vaxd_suscep_distr_shape"];
    suscep_distr_params[VACCINATED][NON_INFLUENZA][SHAPE] = cfg_params["vaxd_suscep_distr_shape"];
    suscep_distr_params[VACCINATED][INFLUENZA][SCALE]     = util::gamma_scale_from_mean(cfg_params["vaxd_suscep_distr_shape"], cfg_params["vaxd_suscep_mean"]);
    suscep_distr_params[VACCINATED][NON_INFLUENZA][SCALE] = util::gamma_scale_from_mean(cfg_params["vaxd_suscep_distr_shape"], cfg_params["vaxd_suscep_mean"]);

    suscep_distr_params[UNVACCINATED][INFLUENZA][SHAPE]     = cfg_params["unvaxd_suscep_distr_shape"];
    suscep_distr_params[UNVACCINATED][NON_INFLUENZA][SHAPE] = cfg_params["unvaxd_suscep_distr_shape"];
    suscep_distr_params[UNVACCINATED][INFLUENZA][SCALE]     = util::gamma_scale_from_mean(cfg_params["unvaxd_suscep_distr_shape"], cfg_params["unvaxd_suscep_mean"]);
    suscep_distr_params[UNVACCINATED][NON_INFLUENZA][SCALE] = util::gamma_scale_from_mean(cfg_params["unvaxd_suscep_distr_shape"], cfg_params["unvaxd_suscep_mean"]);

    vax_effect_distr_params[INFLUENZA][A] = cfg_params["flu_vax_effect_distr_a"];
    vax_effect_distr_params[INFLUENZA][B] = cfg_params["flu_vax_effect_distr_b"];

    pr_symptoms[INFLUENZA]     = cfg_params["pr_symptoms_given_flu"];
    pr_symptoms[NON_INFLUENZA] = cfg_params["pr_symptoms_given_nonflu"];

    pr_seek_care[VACCINATED]   = cfg_params["pr_careseeking_given_vaxd"];
    pr_seek_care[UNVACCINATED] = cfg_params["pr_careseeking_given_unvaxd"];

    pr_exposure[INFLUENZA]     = cfg_params["pr_flu_exposure"];
    pr_exposure[NON_INFLUENZA] = cfg_params["pr_nonflu_exposure"];
    calc_strain_probs();
}

Parameters::~Parameters() {}

void Parameters::init_parameters() {
    population_size = (size_t) 1e3;
    simulation_duration = 200;

    pr_vaccination = 0.5;
    pr_prior_immunity = 1.0;
    pr_exposure = std::vector<double>(NUM_STRAIN_TYPES, 0.01);
    pr_symptoms = std::vector<double>(NUM_STRAIN_TYPES, 1);
    pr_seek_care = std::vector<double>(NUM_VACCINATION_STATUSES, 1);

    suscep_distr_params = std::vector<std::vector<GammaDistrParamArray>>(NUM_VACCINATION_STATUSES,
                                                                         std::vector<GammaDistrParamArray>(NUM_STRAIN_TYPES,
                                                                                                           {0.0, 1.0}));

    vax_effect_distr_params = std::vector<BetaDistrParamArray>(NUM_STRAIN_TYPES, {0.0, 0.0});
    vax_effect_distr_params[INFLUENZA] = {0.0, 0.5};

    strain_probs = std::vector<double>(NUM_STRAIN_TYPES + 1, 0.0);
    calc_strain_probs();

    return_metrics = std::vector<std::string>(0);

    linelist_file_path = "simlinelist.out";
    simvis_file_path   = "simvis.out";
    database_path      = "";
}

void Parameters::calc_strain_probs() {
    for (size_t s = 0; s < NUM_STRAIN_TYPES; ++s) {
        strain_probs[s] = pr_exposure[s];
    }
    strain_probs[NUM_STRAIN_TYPES] = 1.0 - std::accumulate(pr_exposure.begin(), pr_exposure.end(), 0.0);
}

double Parameters::sample_discrete_susceptibility(const GammaDistrParamArray& params) const {
    return (rng->draw_from_rng(INFECTION) < pr_prior_immunity) ? params[SCALE] : 1.0;
}

double Parameters::sample_continuous_susceptibility(const GammaDistrParamArray& params) const {
    return gsl_ran_gamma(rng->get_rng(INFECTION), params[SHAPE], params[SCALE]);
}

std::vector<double> Parameters::sample_susceptibility(const Person* p) const {
    std::vector<double> susceps(NUM_STRAIN_TYPES, 1.0);
    auto vax_specific_suscep_params = p->is_vaccinated() ? suscep_distr_params[VACCINATED] : suscep_distr_params[UNVACCINATED];
    for (size_t strain = 0; strain < NUM_STRAIN_TYPES; ++strain) {
        auto strain_specific_suscep_params = vax_specific_suscep_params[strain];
        susceps[strain] = strain_specific_suscep_params[SHAPE] == 0.0
                              ? sample_discrete_susceptibility(strain_specific_suscep_params)
                              : sample_continuous_susceptibility(strain_specific_suscep_params);
    }

    return susceps;
}

double Parameters::sample_discrete_vaccine_effect(const BetaDistrParamArray& params) const {
    return params[B];
}

double Parameters::sample_continuous_vaccine_effect(const BetaDistrParamArray& params) const {
    return gsl_ran_beta(rng->get_rng(VACCINATION), params[A], params[B]);
}

std::vector<double> Parameters::sample_vaccine_effect() const {
    std::vector<double> vax_effects(NUM_STRAIN_TYPES, 0.0);
    for (size_t strain = 0; strain < NUM_STRAIN_TYPES; ++strain) {
        auto strain_specific_vax_params = vax_effect_distr_params[strain];
        vax_effects[strain] = strain_specific_vax_params[A] == 0.0
                                  ? sample_discrete_vaccine_effect(strain_specific_vax_params)
                                  : sample_continuous_vaccine_effect(strain_specific_vax_params);
    }
    return vax_effects;
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