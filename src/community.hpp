#pragma once

#include <vector>
#include <memory>

#include "parameters.hpp"
#include "utility.hpp"

class Person;
class Infection;
class RngHandler;
class Ledger;

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