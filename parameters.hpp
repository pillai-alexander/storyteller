#pragma once

#include <vector>

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

namespace constants {
    extern unsigned int ONE;
}

class Parameters {
  public:
    Parameters(const RngHandler* rng_handler);
    ~Parameters();

    double sample_susceptibility(const Person* p) const;
    StrainType sample_strain() const;

    void update_time_varying_parameters();

    double pr_vaccination;
    std::vector<double> pr_infection;
    std::vector<double> pr_symptoms;
    std::vector<double> pr_seek_care;

    std::vector<double> strain_probs;

    size_t population_size;
    size_t simulation_duration;
  private:
    const RngHandler* rng;
};