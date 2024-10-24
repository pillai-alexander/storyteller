#pragma once

#include <memory>
#include <string>

#include <gsl/gsl_rng.h>

#include "parameters.hpp"

class Community;
class Infection;

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

  private:
    gsl_rng* infection_rng;
    gsl_rng* vaccination_rng;
    gsl_rng* behavior_rng;
};

class LineList {
  public:
    LineList(const Parameters* parameters);
    ~LineList();

    void generate_linelist_csv(std::string filepath = "");
    void log_infection(const Infection* i);

  private:
    void extract_infection_information();

    std::vector<const Infection*> infections;
    std::vector<size_t> inf_time;
    std::vector<StrainType> inf_strain;
    std::vector<SymptomClass> inf_symptoms;
    std::vector<bool> inf_care;
    std::vector<size_t> person_id;
    std::vector<VaccinationStatus> vax_status;
    std::vector<double> baseline_suscep;
    std::vector<double> vax_effect;

    std::string header;

    const Parameters* par;
};

class Simulator {
  public:
    Simulator();
    ~Simulator();

    void init();
    void simulate();
    void tick();
    LineList results();

  private:
    size_t sim_time;
    unsigned long int rng_seed;
    std::unique_ptr<Parameters> par;
    std::unique_ptr<Community> community;
    std::unique_ptr<RngHandler> rng_handler;
};