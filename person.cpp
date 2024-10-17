#include "person.hpp"
#include "simulator.hpp"
#include "parameters.hpp"

Infection::Infection(StrainType strain, size_t t, SymptomClass sympt, bool care)
    : infection_strain(strain),
      infection_time(t),
      symptoms(sympt),
      sought_care(care) {}

Infection::~Infection() {}

Person::Person(const Parameters* parameters, const RngHandler* rng_handler)
    : susceptibility(1.0),
      infection_history({}),
      vaccination_status(UNVACCINATED) {
    par = parameters;
    rng = rng_handler;
}

Person::~Person() {}

double Person::get_susceptibility() const { return susceptibility; }
void Person::set_susceptibility(double s) { susceptibility = s; }

void Person::infect(StrainType strain, size_t time) {
    auto sympt = (rng->draw_from_rng(INFECTION) < par->pr_symptoms[strain])
        ? SYMPTOMATIC : ASYMPTOMATIC;
    auto seek_care = rng->draw_from_rng(BEHAVIOR) < par->pr_seek_care[vaccination_status];
    infection_history.emplace_back(strain, time, sympt, seek_care);
}

bool Person::vaccinate() {
    if (vaccination_status == VACCINATED) { return false; }
    vaccination_status = VACCINATED;
    return true;
}

bool Person::has_been_infected() const { return infection_history.size() > 0; }
bool Person::is_vaccinated() const { return vaccination_status == VACCINATED; }

void Person::update_susceptibility() {}