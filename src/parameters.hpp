#pragma once

#include <vector>
#include <array>

class Person;
class RngHandler;

enum StrainType {
    NON_INFLUENZA,
    INFLUENZA,
    NUM_STRAIN_TYPES
};

enum VaccinationStatus {
    UNVACCINATED,
    VACCINATED,
    NUM_VACCINATION_STATUSES
};

enum SymptomClass {
    ASYMPTOMATIC,
    SYMPTOMATIC,
    NUM_SYMPTOM_CLASSES
};

enum BetaDistributionParameter {
    A,
    B,
    NUM_BETA_DISTR_PARAMS
};

enum GammaDistributionParameter {
    SHAPE,
    RATE,
    NUM_GAMMA_DISTR_PARAMS
};

namespace constants {
    extern unsigned int ONE;
}

class Parameters {
  public:
    Parameters(const RngHandler* rng_handler);
    ~Parameters();

    double sample_susceptibility(const Person* p) const;
    double sample_vaccine_effect() const;
    StrainType sample_strain() const;

    void update_time_varying_parameters();

    double pr_vaccination;
    std::vector<double> pr_exposure;
    std::vector<double> pr_symptoms;
    std::vector<double> pr_seek_care;

    std::vector<double> strain_probs;

    double baseline_suscep_distr_shape;
    std::vector<double> baseline_suscep_distr_mean;

    std::array<double, NUM_BETA_DISTR_PARAMS> vax_effect_distr_params;

    size_t population_size;
    size_t simulation_duration;
  private:
    void init_parameters();
    const RngHandler* rng;
};