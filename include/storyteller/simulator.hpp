#pragma once

#include <memory>
#include <string>
#include <map>

#include <gsl/gsl_rng.h>

#include "parameters.hpp"

class Community;
class Infection;
class Ledger;
class DatabaseHandler;
class RngHandler;

class Simulator {
  public:
    Simulator(const Parameters* parameters, const DatabaseHandler* dbh, const RngHandler* rngh);
    ~Simulator();

    void set_flags(std::map<std::string, bool> flags);

    void init();
    void simulate();
    void tick();
    void results();

  private:
    size_t sim_time;
    std::map<std::string, bool> sim_flags;

    std::unique_ptr<Community> community;
    const RngHandler* rng_handler;
    const Parameters* par;
    const DatabaseHandler* db_handler;
};