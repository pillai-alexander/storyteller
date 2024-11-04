/**
 * @file person.hpp
 * @author Alexander N. Pillai
 * @brief Contains the Storyteller class that is responsible for taking in user
 *        input and performing the desired operations (eg, database initialization,
 *        simulations)
 *
 * @copyright TBD
 */
#pragma once

#include <vector>
#include <memory>
#include <iostream>

#include "parameters.hpp"

/**
 * @brief Represents a single infection event for a Person and stores all relevant
 *        infection information.
 */
class Infection {
  friend class Person;
  public:
    Infection(Person* p, StrainType strain, size_t time, SymptomClass sympt, bool care);
    ~Infection();

    Person* get_infectee() const;
    StrainType get_strain() const;
    size_t get_infection_time() const;
    SymptomClass get_symptoms() const;
    bool get_sought_care() const;

  private:
    Person* infectee;
    StrainType infection_strain;
    size_t infection_time;
    SymptomClass symptoms;
    bool sought_care;
};

/**
 * @brief Primary agent of the simulation that is stored in a Community.
 */
class Person {
  public:
    Person(size_t id, const Parameters* parameters, const RngHandler* rng_handler);
    ~Person();

    size_t get_id() const;

    double get_susceptibility(StrainType strain) const;
    void set_susceptibility(StrainType strain, double s);

    double get_vaccine_protection(StrainType strain) const;
    void set_vaccine_protection(StrainType strain, double vp);

    const std::vector<std::unique_ptr<Infection>>& get_infection_history() const;

    Infection* attempt_infection(StrainType strain, size_t time);
    bool vaccinate();

    bool has_been_infected() const;
    bool has_been_infected_with(StrainType strain) const;
    bool is_vaccinated() const;
    bool is_susceptible_to(StrainType strain) const;

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