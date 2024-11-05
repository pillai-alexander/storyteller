/**
 * @file ledger.cpp
 * @author Alexander N. Pillai
 * @brief Contains the Ledger class that stores simulation data during the core
 *        simulation loop and pre-processes the data after the simulation ends.
 *
 * @copyright TBD
 */
#include <numeric>
#include <iostream>
#include <fstream>
#include <math.h>

#include <storyteller/ledger.hpp>
#include <storyteller/person.hpp>

Ledger::Ledger(const Parameters* parameters) {
    par = parameters;
    inf_incidence       = vector3d<size_t>(NUM_VACCINATION_STATUSES, vector2d<size_t>(NUM_STRAIN_TYPES, std::vector<size_t>(par->simulation_duration, 0)));
    sympt_inf_incidence = vector3d<size_t>(NUM_VACCINATION_STATUSES, vector2d<size_t>(NUM_STRAIN_TYPES, std::vector<size_t>(par->simulation_duration, 0)));
    mai_incidence       = vector3d<size_t>(NUM_VACCINATION_STATUSES, vector2d<size_t>(NUM_STRAIN_TYPES, std::vector<size_t>(par->simulation_duration, 0)));

    cumul_infs       = vector3d<size_t>(NUM_VACCINATION_STATUSES, vector2d<size_t>(NUM_STRAIN_TYPES, std::vector<size_t>(par->simulation_duration, 0)));
    cumul_sympt_infs = vector3d<size_t>(NUM_VACCINATION_STATUSES, vector2d<size_t>(NUM_STRAIN_TYPES, std::vector<size_t>(par->simulation_duration, 0)));
    cumul_mais       = vector3d<size_t>(NUM_VACCINATION_STATUSES, vector2d<size_t>(NUM_STRAIN_TYPES, std::vector<size_t>(par->simulation_duration, 0)));

    tnd_ve_estimate = std::vector<double>(par->simulation_duration, 0.0);

    vax_incidence = std::vector<size_t>(par->simulation_duration, 0);

    linelist_header = "inf_id,inf_time,inf_strain,inf_sympts,inf_care,p_id,vax_status,baseline_suscep,vax_effect";
    simvis_header = "time,vaxd_flu_infs,vaxd_flu_mais,vaxd_nonflu_infs,vaxd_nonflu_mais,unvaxd_flu_infs,unvaxd_flu_mais,unvaxd_nonflu_infs,unvaxd_nonflu_mais,tnd_ve_est";
}

Ledger::~Ledger() {}

vector3d<size_t> Ledger::get_inf_incidence() const { return inf_incidence; }
vector3d<size_t> Ledger::get_sympt_inf_incidence() const { return sympt_inf_incidence; }
vector3d<size_t> Ledger::get_mai_incidence() const { return mai_incidence; }

vector3d<size_t> Ledger::get_cumul_infs() const { return cumul_infs;}
vector3d<size_t> Ledger::get_cumul_sympt_infs() const { return cumul_sympt_infs;}
vector3d<size_t> Ledger::get_cumul_mais() const { return cumul_mais;}

std::vector<size_t> Ledger::get_vax_incidence() const { return vax_incidence; }
std::vector<double> Ledger::get_tnd_ve_est() const { return tnd_ve_estimate; }

size_t Ledger::get_cumul_infs(VaccinationStatus vaxd, StrainType strain, size_t time) const { return cumul_infs[vaxd][strain][time]; }
size_t Ledger::get_cumul_sympt_infs(VaccinationStatus vaxd, StrainType strain, size_t time) const { return cumul_sympt_infs[vaxd][strain][time]; }
size_t Ledger::get_cumul_mais(VaccinationStatus vaxd, StrainType strain, size_t time) const { return cumul_mais[vaxd][strain][time]; }
double Ledger::get_tnd_ve_est(size_t time) const { return tnd_ve_estimate[time]; }

void Ledger::log_infection(const Infection* i) {
    auto vaxd   = i->get_infectee()->is_vaccinated();
    auto time   = i->get_infection_time();
    auto strain = i->get_strain();
    auto sympts = i->get_symptoms();
    auto mai    = i->get_sought_care();

    infections.push_back(i);

    inf_incidence[vaxd][strain][time]++;
    if (sympts == SYMPTOMATIC) sympt_inf_incidence[vaxd][strain][time]++;
    if (mai) mai_incidence[vaxd][strain][time]++;
}

size_t Ledger::total_infections(VaccinationStatus vaxd, StrainType strain) const {
    return std::accumulate(inf_incidence[vaxd][strain].begin(),
                                 inf_incidence[vaxd][strain].end(),
                                 0);
}

size_t Ledger::total_sympt_infections(VaccinationStatus vaxd, StrainType strain) const {
    return std::accumulate(sympt_inf_incidence[vaxd][strain].begin(),
                           sympt_inf_incidence[vaxd][strain].end(),
                           0);
}

size_t Ledger::total_mai(VaccinationStatus vaxd, StrainType strain) const {
    return std::accumulate(mai_incidence[vaxd][strain].begin(),
                           mai_incidence[vaxd][strain].end(),
                           0);
}

size_t Ledger::total_vaccinations() const {
    return std::accumulate(vax_incidence.begin(),
                           vax_incidence.end(),
                           0);
}

void Ledger::calculate_cumulatives() {
    for (size_t v = 0; v < NUM_VACCINATION_STATUSES; ++v) {
        for (size_t s = 0; s < NUM_STRAIN_TYPES; ++s) {
            std::partial_sum(inf_incidence[v][s].cbegin(), inf_incidence[v][s].cend(), cumul_infs[v][s].begin());
            std::partial_sum(sympt_inf_incidence[v][s].cbegin(), sympt_inf_incidence[v][s].cend(), cumul_sympt_infs[v][s].begin());
            std::partial_sum(mai_incidence[v][s].cbegin(), mai_incidence[v][s].cend(), cumul_mais[v][s].begin());
        }
    }
}

void Ledger::calculate_tnd_ve_est() {
    for (size_t t = 0; t < par->simulation_duration; ++t) {
        auto cumul_vax_flu_mais      = cumul_mais[VACCINATED][INFLUENZA][t];
        auto cumul_vax_nonflu_mais   = cumul_mais[VACCINATED][NON_INFLUENZA][t];
        auto cumul_unvax_flu_mais    = cumul_mais[UNVACCINATED][INFLUENZA][t];
        auto cumul_unvax_nonflu_mais = cumul_mais[UNVACCINATED][NON_INFLUENZA][t];
    
        auto flu_vax_odds    = ((double) cumul_vax_flu_mais) / cumul_unvax_flu_mais;
        auto nonflu_vax_odds = ((double) cumul_vax_nonflu_mais) / cumul_unvax_nonflu_mais;
        double ve_est        = 1 - (flu_vax_odds / nonflu_vax_odds);

        tnd_ve_estimate[t] = isfinite(ve_est) ? ve_est : 0.0;
    }
}

void Ledger::generate_linelist_csv(std::string filepath) {
    if (filepath.empty()) filepath = par->linelist_file_path;
    std::ofstream file(filepath);
    file << linelist_header << '\n';
    for (size_t i = 0; i < infections.size(); ++i) {
        auto inf = infections[i];
        auto strain = inf->get_strain();
        auto infectee = inf->get_infectee();
        file << i << ','
             << inf->get_infection_time() << ','
             << strain << ','
             << inf->get_symptoms() << ','
             << inf->get_sought_care() << ','
             << infectee->get_id() << ','
             << infectee->is_vaccinated() << ','
             << infectee->get_susceptibility(strain) << ','
             << infectee->get_vaccine_protection(strain) << '\n';
    }
    file.close();
}

void Ledger::generate_simvis_csv(std::string filepath) {
    if (filepath.empty()) filepath = par->simvis_file_path;
    std::ofstream file(filepath);
    file << simvis_header << '\n';
    for (size_t t = 0; t < par->simulation_duration; ++t) {
        file << t << ','
             << inf_incidence[VACCINATED][INFLUENZA][t] << ','
             << mai_incidence[VACCINATED][INFLUENZA][t] << ','
             << inf_incidence[VACCINATED][NON_INFLUENZA][t] << ','
             << mai_incidence[VACCINATED][NON_INFLUENZA][t] << ','
             << inf_incidence[UNVACCINATED][INFLUENZA][t] << ','
             << mai_incidence[UNVACCINATED][INFLUENZA][t] << ','
             << inf_incidence[UNVACCINATED][NON_INFLUENZA][t] << ','
             << mai_incidence[UNVACCINATED][NON_INFLUENZA][t] << ','
             << tnd_ve_estimate[t] << '\n';

    }
    file.close();
}