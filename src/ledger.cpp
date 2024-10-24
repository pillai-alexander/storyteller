#include <numeric>
#include <iostream>
#include <fstream>

#include "ledger.hpp"
#include "person.hpp"

Ledger::Ledger(const Parameters* parameters) {
    par = parameters;
    inf_incidence       = vector3d<size_t>(NUM_VACCINATION_STATUSES, vector2d<size_t>(NUM_STRAIN_TYPES, std::vector<size_t>(par->simulation_duration, 0)));
    sympt_inf_incidence = vector3d<size_t>(NUM_VACCINATION_STATUSES, vector2d<size_t>(NUM_STRAIN_TYPES, std::vector<size_t>(par->simulation_duration, 0)));
    mai_incidence       = vector3d<size_t>(NUM_VACCINATION_STATUSES, vector2d<size_t>(NUM_STRAIN_TYPES, std::vector<size_t>(par->simulation_duration, 0)));

    vax_incidence = std::vector<size_t>(par->simulation_duration, 0);

    linelist_header = "inf_id,inf_time,inf_strain,inf_sympts,inf_care,p_id,vax_status,baseline_suscep,vax_effect";
}

Ledger::~Ledger() {}

vector3d<size_t> Ledger::get_inf_incidence() const { return inf_incidence; }
vector3d<size_t> Ledger::get_sympt_inf_incidence() const { return sympt_inf_incidence; }
vector3d<size_t> Ledger::get_mai_incidence() const { return mai_incidence; }
std::vector<size_t> Ledger::get_vax_incidence() const { return vax_incidence; }

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