#pragma once

#include <vector>
#include <array>
#include <string>
#include <map>

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

typedef std::array<double, NUM_BETA_DISTR_PARAMS> BetaDistrParamArray;

enum GammaDistributionParameter {
    SHAPE,
    SCALE,
    NUM_GAMMA_DISTR_PARAMS
};

typedef std::array<double, NUM_GAMMA_DISTR_PARAMS> GammaDistrParamArray;

class Parameters {
  public:
    Parameters(const RngHandler* rng_handler);
    Parameters(const RngHandler* rng_handler, std::map<std::string, double> cfg_params);
    ~Parameters();

    std::vector<double> sample_susceptibility(const Person* p) const;
    std::vector<double> sample_vaccine_effect() const;
    StrainType sample_strain() const;

    void update_time_varying_parameters();

    double pr_vaccination;
    std::vector<double> pr_exposure;
    std::vector<double> pr_symptoms;
    std::vector<double> pr_seek_care;

    std::vector<double> strain_probs;

    std::vector<std::vector<GammaDistrParamArray>> suscep_distr_params;
    std::vector<BetaDistrParamArray> vax_effect_distr_params;

    size_t population_size;
    size_t simulation_duration;

    std::string linelist_file_path;
    std::string simvis_file_path;
    std::string database_path;
    size_t simulation_serial;

    std::vector<std::string> return_metrics;

  private:
    void init_parameters();

    const RngHandler* rng;
};