/**
 * @file utility.hpp
 * @author Alexander N. Pillai
 * @brief Contains the global utility variables and methods and the RngHandler
 *        and DatabaseHandler classes.
 *
 * @copyright TBD
 */
#pragma once

#include <vector>

#include <gsl/gsl_rng.h>

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

    extern double beta_a_from_mean_var(double mean, double var);
    extern double beta_b_from_mean_var(double mean, double var);

    extern double logistic(const double log_odds);
    extern double logit(const double prob);
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
    RngHandler();
    ~RngHandler();

    void set_seed(const unsigned long int seed);

    double draw_from_rng(RngType type = INFECTION) const;
    gsl_rng* get_rng(RngType type = INFECTION) const;

    unsigned long int get_seed() const;

  private:
    unsigned long int rng_seed;
    gsl_rng* infection_rng;
    gsl_rng* vaccination_rng;
    gsl_rng* behavior_rng;
};