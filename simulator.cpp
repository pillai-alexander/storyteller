#include <memory>
#include <iostream>

#include <gsl/gsl_rng.h>

#include "simulator.hpp"
#include "community.hpp"
#include "parameters.hpp"

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

Simulator::Simulator()
    : sim_time(0),
      rng_handler(std::make_unique<RngHandler>()) {
    par = std::make_unique<Parameters>(rng_handler.get());
    community = std::make_unique<Community>(par.get(), rng_handler.get());
}

Simulator::~Simulator() {}

void Simulator::simulate() {
    for (; sim_time < par->simulation_duration; ++sim_time) {
        tick();
    }
}

void Simulator::tick() {
    // par->update_time_varying_parameters();
    community->transmission(sim_time);
}

void Simulator::results() {
    std::cout << "flu infs: " << community->cumulative_infections[INFLUENZA] << std::endl;
    std::cout << "nonflu infs: " << community->cumulative_infections[NON_INFLUENZA] << std::endl;
}