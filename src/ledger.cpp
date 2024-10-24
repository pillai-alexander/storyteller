#include <numeric>

#include "ledger.hpp"
#include "person.hpp"

Ledger::Ledger(const Parameters* parameters) {
    par = parameters;
    inf_incidence       = vector3d<size_t>(NUM_VACCINATION_STATUSES, vector2d<size_t>(NUM_STRAIN_TYPES, std::vector<size_t>(par->simulation_duration, 0)));
    sympt_inf_incidence = vector3d<size_t>(NUM_VACCINATION_STATUSES, vector2d<size_t>(NUM_STRAIN_TYPES, std::vector<size_t>(par->simulation_duration, 0)));
    mai_incidence       = vector3d<size_t>(NUM_VACCINATION_STATUSES, vector2d<size_t>(NUM_STRAIN_TYPES, std::vector<size_t>(par->simulation_duration, 0)));

    vax_incidence = std::vector<size_t>(par->simulation_duration, 0);
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