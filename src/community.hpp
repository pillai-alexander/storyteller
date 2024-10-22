#pragma once

#include <vector>
#include <memory>

class Person;
class Parameters;
class RngHandler;

class Community {
  friend class Simulator;
  public:
    Community(const Parameters* parameters, const RngHandler* rng_handler);
    ~Community();

    void transmission(size_t time);

    std::vector<size_t> cumulative_infections;

  private:
    void init_population();
    void vaccinate_population();
    void init_susceptibilities();
    
    std::vector<std::unique_ptr<Person>> people;
    std::vector<Person*> susceptibles;
    const Parameters* par;
    const RngHandler* rng;
};