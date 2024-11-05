/**
 * @file utility.cpp
 * @author Alexander N. Pillai
 * @brief Contains the global utility variables and methods and the RngHandler
 *        and DatabaseHandler classes.
 *
 * @copyright TBD
 */
#include <algorithm>


#include <storyteller/utility.hpp>
#include <storyteller/simulator.hpp>
#include <storyteller/storyteller.hpp>

namespace constants {
    unsigned int ZERO = 0;
    unsigned int ONE = 1;
}

namespace util {
    double gamma_scale_from_mean(double shape, double mean) {
        return (shape == constants::ZERO) ? mean : mean / shape;
    }

    // iterative odometer-like combinations of multiple vectors
    // https://stackoverflow.com/questions/1700079/howto-create-combinations-of-several-vectors-without-hardcoding-loops-in-c
    // https://stackoverflow.com/questions/5279051/how-can-i-create-the-cartesian-product-of-a-vector-of-vectors
    vector2d<double> vec_combinations(vector2d<double> vecs) {
        size_t n_vectors = vecs.size();
        std::vector<std::vector<double>::const_iterator> its(n_vectors);
        for (size_t i = 0; i < n_vectors; ++i) {
            its[i] = vecs[i].cbegin();
        }

        std::vector<std::vector<double>> out;
        while (its[0] != vecs[0].cend()) {
            std::vector<double> row;
            for (const auto& it : its) {
                row.push_back(*it);
            }
            out.push_back(row);

            ++its[n_vectors - 1];
            for (size_t i = n_vectors - 1; (i > 0) and (its[i] == vecs[i].cend()); --i) {
                its[i] = vecs[i].cbegin();
                ++its[i - 1];
            }
        }
        return out;
    }
}

RngHandler::RngHandler(unsigned long int seed) : rng_seed(seed) {
    infection_rng   = gsl_rng_alloc(gsl_rng_mt19937);
    vaccination_rng = gsl_rng_alloc(gsl_rng_mt19937);
    behavior_rng    = gsl_rng_alloc(gsl_rng_mt19937);

    gsl_rng_set(infection_rng, seed);
    gsl_rng_set(vaccination_rng, seed);
    gsl_rng_set(behavior_rng, seed);
}

RngHandler::~RngHandler() {
    gsl_rng_free(infection_rng);
    gsl_rng_free(vaccination_rng);
    gsl_rng_free(behavior_rng);
}

double RngHandler::draw_from_rng(RngType type) const {
    switch (type) {
        case INFECTION:   { return gsl_rng_uniform(infection_rng); }
        case VACCINATION: { return gsl_rng_uniform(vaccination_rng); }
        case BEHAVIOR:    { return gsl_rng_uniform(behavior_rng); }
        default:          { return gsl_rng_uniform(infection_rng); }
    }
}

gsl_rng* RngHandler::get_rng(RngType type) const {
    switch (type) {
        case INFECTION:   { return infection_rng; }
        case VACCINATION: { return vaccination_rng; }
        case BEHAVIOR:    { return behavior_rng; }
        default:          { return infection_rng; }
    }
}

unsigned long int RngHandler::get_seed() const { return rng_seed; }