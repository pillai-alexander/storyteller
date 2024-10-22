#pragma once

#include <vector>
#include <memory>

#include "parameters.hpp"

class Infection {
  friend class Person;
  public:
    Infection(StrainType strain, size_t time, SymptomClass sympt, bool care);
    ~Infection();

    StrainType get_strain() const;
    size_t get_infection_time() const;
    SymptomClass get_symptoms() const;
    bool get_sought_care() const;

  private:
    StrainType infection_strain;
    size_t infection_time;
    SymptomClass symptoms;
    bool sought_care;
};

class Person {
  public:
    Person(const Parameters* parameters, const RngHandler* rng_handler);
    ~Person();

    double get_susceptibility() const;
    void set_susceptibility(double s);

    bool infect(StrainType strain, size_t time);
    bool vaccinate();

    bool has_been_infected() const;
    bool is_vaccinated() const;

    Infection* most_recent_infection() const;

  private:
    void update_susceptibility();

    double susceptibility;
    double vaccine_protection;
    std::vector<std::unique_ptr<Infection>> infection_history;
    VaccinationStatus vaccination_status;
    const Parameters* par;
    const RngHandler* rng;
};