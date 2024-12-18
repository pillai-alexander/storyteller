/**
 * @file utility.cpp
 * @author Alexander N. Pillai
 * @brief Contains the global utility variables and methods and the RngHandler
 *        and DatabaseHandler classes.
 *
 * @copyright TBD
 */
#include <algorithm>
#include <cmath>


#include <storyteller/utility.hpp>
#include <storyteller/simulator.hpp>
#include <storyteller/storyteller.hpp>

namespace constants {
    unsigned int ZERO = 0;
    unsigned int ONE = 1;
    double PI = 3.14159265358979323846;
}

namespace util {
    double gamma_scale_from_mean(double shape, double mean) {
        return (shape == constants::ZERO) ? mean : mean / shape;
    }

    // iterative odometer-like combinations of multiple vectors
    // https://stackoverflow.com/questions/1700079/howto-create-combinations-of-several-vectors-without-hardcoding-loops-in-c
    // https://stackoverflow.com/questions/5279051/how-can-i-create-the-cartesian-product-of-a-vector-of-vectors
    vector2d<double> vec_combinations(vector2d<double> vecs) {
        std::vector<std::vector<double>> out;
        size_t n_vectors = vecs.size();
        if (n_vectors == 0) return out;

        std::vector<std::vector<double>::const_iterator> its(n_vectors);
        for (size_t i = 0; i < n_vectors; ++i) {
            its[i] = vecs[i].cbegin();
        }

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

    double beta_a_from_mean_var(double mean, double var) {
        auto max_var = mean * (1 - mean);
        return (var < max_var) ? mean * ((max_var / var) - 1.0) : -1.0;
    }

    double beta_b_from_mean_var(double mean, double var) {
        auto max_var = mean * (1 - mean);
        return (var < max_var) ? (1 - mean) * ((max_var / var) - 1.0) : -1.0;
    }

    double logistic(const double log_odds) {
        return 1 / (1 + std::exp(-1.0 * log_odds));
    }

    double logit(const double prob) {
        return std::log(prob / (1 - prob));
    }

    double exp_decay_rate_from_half_life(const double half_life) {
        return std::log(2) / half_life;
    }

    double exp_decay(const double rate, const double time) {
        return std::exp(-1 * rate * time);
    }
}

RngHandler::RngHandler() {
    infection_rng   = gsl_rng_alloc(gsl_rng_mt19937);
    vaccination_rng = gsl_rng_alloc(gsl_rng_mt19937);
    behavior_rng    = gsl_rng_alloc(gsl_rng_mt19937);
}

RngHandler::~RngHandler() {
    gsl_rng_free(infection_rng);
    gsl_rng_free(vaccination_rng);
    gsl_rng_free(behavior_rng);
}

void RngHandler::set_seed(const unsigned long int seed) {
    rng_seed = seed;
    gsl_rng_set(infection_rng, seed);
    gsl_rng_set(vaccination_rng, seed);
    gsl_rng_set(behavior_rng, seed);
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