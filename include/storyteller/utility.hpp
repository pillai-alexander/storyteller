#pragma once

#include <string>
#include <vector>
#include <map>

#include <gsl/gsl_rng.h>

class Storyteller;
class Ledger;
class Parameters;

namespace constants {
    extern unsigned int ONE;
}

template<typename T> using vector2d = std::vector<std::vector<T>>;
template<typename T> using vector3d = std::vector<std::vector<std::vector<T>>>;

namespace util {
    extern double gamma_scale_from_mean(double shape, double mean);
}

enum RngType {
    INFECTION,
    VACCINATION,
    BEHAVIOR,
    NUM_RNG_TYPES
};

class RngHandler {
  public:
    RngHandler(unsigned long int seed);
    ~RngHandler();

    double draw_from_rng(RngType type = INFECTION) const;
    gsl_rng* get_rng(RngType type = INFECTION) const;

    unsigned long int get_seed() const;

  private:
    unsigned long int rng_seed;
    gsl_rng* infection_rng;
    gsl_rng* vaccination_rng;
    gsl_rng* behavior_rng;
};

class DatabaseHandler {
  public:
    DatabaseHandler(const Storyteller* storyteller, std::string db_path);
    ~DatabaseHandler();

    void init_database();
    void create_table();
    void clear_table();

    bool database_exists();
    bool table_exists();

    void read_parameters(unsigned int serial, std::map<std::string, double>& pars);
    void write_metrics(const Ledger* ledger, const Parameters* par) const;

  private:
    std::vector<std::string> prepare_insert_sql(const Ledger* ledger, const Parameters* par) const;

    std::string database_path;
    size_t n_transaction_attempts;
    size_t ms_delay_between_attempts;

    const Storyteller* owner;
};