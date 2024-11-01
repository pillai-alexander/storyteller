/**
 * @file utility.hpp
 * @author Alexander N. Pillai
 * @brief Contains the global utility variables and methods and the RngHandler
 *        and DatabaseHandler classes.
 *
 * @copyright TBD
 */
#pragma once

#include <string>
#include <vector>
#include <map>

#include <gsl/gsl_rng.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Storyteller;
class Ledger;
class Parameters;

/**
 * @brief Contains any useful global constants.
 */
namespace constants {
    extern unsigned int ZERO;
    extern unsigned int ONE;
}

/**
 * @brief General two-dimensional vector that stores type T values.
 * 
 * @tparam T Type of the value stored
 */
template<typename T> using vector2d = std::vector<std::vector<T>>;

/**
 * @brief General three-dimensional vector that stores type T values.
 * 
 * @tparam T Type of the value stored
 */
template<typename T> using vector3d = std::vector<std::vector<std::vector<T>>>;

/**
 * @brief Contains any useful utility functions.
 */
namespace util {
    /**
     * @brief Calculates the gamma distribution scale parameter from the shape
     *        and mean.
     * 
     * @param shape Gamma distribution shape parameter
     * @param mean Gamma distribution mean
     * @return double Gamma distribution scale parameter
     */
    extern double gamma_scale_from_mean(double shape, double mean);

    /**
     * @brief Calculate all combinations of the elements of provided vectors.
     * 
     * @param vecs Vector of the vectors to create combinations from
     * @return vector2d<double> Vector of calculated combinations
     */
    extern vector2d<double> vec_combinations(vector2d<double> vecs);
}

/**
 * @brief Defines the types of pseudo-random number generators that the RngHandler
 *        will store.
 */
enum RngType {
    INFECTION,
    VACCINATION,
    BEHAVIOR,
    NUM_RNG_TYPES
};

/**
 * @brief Handles all pseudo-random number generation and related operations.
 * 
 * Will store a separate pseudo-random number generator for each RngType and will
 * initialize each generator with the seed provided by the Storyteller.
 */
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

/**
 * @brief Handles all SQLite database operations.
 * 
 * Includes methods that create a new experiment database using the user-provided
 * configuration file, read simulation parameters for a specific particle, and
 * write simulation metrics to the database after a simulation terminates.
 */
class DatabaseHandler {
  public:
    DatabaseHandler(const Storyteller* storyteller, std::string db_path);
    ~DatabaseHandler();

    int init_database(json cfg);
    void create_table();
    void clear_table();

    bool database_exists();
    bool table_exists(std::string table);

    void read_parameters(unsigned int serial, std::map<std::string, double>& pars);
    void write_metrics(const Ledger* ledger, const Parameters* par) const;

  private:
    std::vector<std::string> prepare_insert_sql(const Ledger* ledger, const Parameters* par) const;

    std::string database_path;
    size_t n_transaction_attempts;
    size_t ms_delay_between_attempts;

    // const Storyteller* owner;
};