#include <memory>

#include "person.hpp"
#include "simulator.hpp"
#include "parameters.hpp"

Infection::Infection(StrainType strain, size_t t, SymptomClass sympt, bool care)
    : infection_strain(strain),
      infection_time(t),
      symptoms(sympt),
      sought_care(care) {}

Infection::~Infection() {}

StrainType Infection::get_strain() const { return infection_strain; }

size_t Infection::get_infection_time() const { return infection_time; }

SymptomClass Infection::get_symptoms() const { return symptoms; }

bool Infection::get_sought_care() const { return sought_care; }

Person::Person(size_t assigned_id, const Parameters* parameters, const RngHandler* rng_handler) {
    id = assigned_id;
    par = parameters;
    rng = rng_handler;

    vaccination_status = UNVACCINATED;
    vaccine_protection = std::vector<double>(NUM_STRAIN_TYPES, 0.0);

    susceptibility = par->sample_susceptibility(this);
}

Person::~Person() {}

double Person::get_susceptibility(StrainType strain) const { return susceptibility[strain]; }
void Person::set_susceptibility(StrainType strain, double s) { susceptibility[strain] = s; }

double Person::get_vaccine_protection(StrainType strain) const { return vaccine_protection[strain]; }
void Person::set_vaccine_protection(StrainType strain, double vp) { vaccine_protection[strain] = vp; }

Infection* Person::infect(StrainType strain, size_t time) {
    auto current_suscep = susceptibility[strain];
    current_suscep *= is_vaccinated() ? 1 - vaccine_protection[strain] : 1;

    if (rng->draw_from_rng(INFECTION) < current_suscep) {
        auto sympt = (rng->draw_from_rng(INFECTION) < par->pr_symptoms[strain]) ? SYMPTOMATIC : ASYMPTOMATIC;
        auto seek_care = rng->draw_from_rng(BEHAVIOR) < par->pr_seek_care[vaccination_status];
        infection_history.push_back(std::make_unique<Infection>(strain, time, sympt, seek_care));

        return infection_history.back().get();
    } else {
        return nullptr;
    }
}

bool Person::vaccinate() {
    if (vaccination_status == VACCINATED) { return false; }
    vaccination_status = VACCINATED;
    vaccine_protection = par->sample_vaccine_effect();
    susceptibility = par->sample_susceptibility(this);
    return true;
}

bool Person::has_been_infected() const { return infection_history.size() > 0; }
bool Person::is_vaccinated() const { return vaccination_status == VACCINATED; }

Infection* Person::most_recent_infection() const {
    auto inf = has_been_infected() ? infection_history.back().get() : nullptr;
    return inf;
}

void Person::update_susceptibility() {}