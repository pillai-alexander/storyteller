#pragma once

#include <vector>
#include <memory>

#include "parameters.hpp"

class Person;
class Infection;
class RngHandler;

class Ledger {
  friend class Community;
  public:
    Ledger(const Parameters* parameters);
    ~Ledger();

    std::vector<std::vector<size_t>> get_inf_incidence() const;
    std::vector<std::vector<size_t>> get_sympt_inf_incidence() const;
    std::vector<std::vector<size_t>> get_mai_incidence() const;
    std::vector<size_t> get_vax_incidence() const;

    void log_infection(const Infection* i);

    size_t total_infections(StrainType strain) const;
    size_t total_sympt_infections(StrainType strain) const;
    size_t total_mai(StrainType strain) const;
    size_t total_vaccinations() const;

  private:
  // EPIDEMIC DATA
    // infection incidence [strain][time]
    std::vector<std::vector<size_t>> inf_incidence;
    // symptomatic infection incidence [strain][time]
    std::vector<std::vector<size_t>> sympt_inf_incidence;
    // mai incidence [strain][time]
    std::vector<std::vector<size_t>> mai_incidence;

    // POPULATION DATA
    // num vaccinated [time]
    std::vector<size_t> vax_incidence;

    const Parameters* par;
};

class Community {
  friend class Simulator;
  public:
    Community(const Parameters* parameters, const RngHandler* rng_handler);
    ~Community();

    void transmission(size_t time);

  private:
    void init_population();
    void vaccinate_population(size_t time);
    void init_susceptibilities();
    
    std::vector<std::unique_ptr<Person>> people;
    std::vector<Person*> susceptibles;

    std::unique_ptr<Ledger> ledger;
    const Parameters* par;
    const RngHandler* rng;
};