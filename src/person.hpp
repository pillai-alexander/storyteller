#pragma once

#include <vector>
#include <memory>
#include <iostream>

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
    Person(size_t id, const Parameters* parameters, const RngHandler* rng_handler);
    ~Person();

    double get_susceptibility(StrainType strain) const;
    void set_susceptibility(StrainType strain, double s);

    double get_vaccine_protection(StrainType strain) const;
    void set_vaccine_protection(StrainType strain, double vp);

    Infection* infect(StrainType strain, size_t time);
    bool vaccinate();

    bool has_been_infected() const;
    bool is_vaccinated() const;

    Infection* most_recent_infection() const;

    friend std::ostream& operator<<(std::ostream& o , const Person& p);

  private:
    void update_susceptibility();

    size_t id;
    std::vector<double> susceptibility;
    std::vector<double> vaccine_protection;
    std::vector<std::unique_ptr<Infection>> infection_history;
    VaccinationStatus vaccination_status;
    const Parameters* par;
    const RngHandler* rng;
};