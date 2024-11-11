/**
 * @file community.hpp
 * @author Alexander N. Pillai
 * @brief Contains the Community object that stores the synthetic population and
 *        performs simulation operations to that population.
 *
 * @copyright TBD
 */
#pragma once

#include <vector>
#include <memory>

class Person;
class Parameters;
class RngHandler;
class Ledger;

/**
 * @brief Object that stores and manipulates a synthetic population for a single
 *        simulation.
 */
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