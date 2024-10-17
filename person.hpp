#pragma once

#include <vector>
#include <memory>

#include "parameters.hpp"

class Infection {
  public:
    Infection(StrainType strain, size_t time, SymptomClass sympt, bool care);
    ~Infection();

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

    void infect(StrainType strain, size_t time);
    bool vaccinate();

    bool has_been_infected() const;
    bool is_vaccinated() const;

  private:
    void update_susceptibility();

    double susceptibility;
    std::vector<Infection> infection_history;
    VaccinationStatus vaccination_status;
    const Parameters* par;
    const RngHandler* rng;
};