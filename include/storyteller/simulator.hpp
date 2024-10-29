#pragma once

#include <memory>
#include <string>
#include <map>

#include <gsl/gsl_rng.h>

#include "parameters.hpp"

class Community;
class Infection;

class Simulator {
  public:
    Simulator();
    Simulator(std::string cfg, size_t serial);
    ~Simulator();

    void set_flags(std::map<std::string, bool> flags);

    void init();
    void simulate();
    void tick();
    void results();

    void write_metrics_to_database();

  private:
    size_t sim_time;
    unsigned long int rng_seed;
    std::map<std::string, bool> sim_flags;

    std::unique_ptr<Parameters> par;
    std::unique_ptr<Community> community;
    std::unique_ptr<RngHandler> rng_handler;
};