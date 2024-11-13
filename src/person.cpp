/**
 * @file person.cpp
 * @author Alexander N. Pillai
 * @brief Contains the Storyteller class that is responsible for taking in user
 *        input and performing the desired operations (eg, database initialization,
 *        simulations)
 *
 * @copyright TBD
 */
#include <memory>
#include <iostream>
#include <algorithm>

#include <storyteller/person.hpp>
#include <storyteller/simulator.hpp>
#include <storyteller/parameters.hpp>
#include <storyteller/utility.hpp>

Infection::Infection(Person* p, StrainType strain, size_t t, SymptomClass sympt, bool care)
    : infectee(p),
      infection_strain(strain),
      infection_time(t),
      symptoms(sympt),
      sought_care(care) {}

Infection::~Infection() {}

Person* Infection::get_infectee() const { return infectee; }
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

size_t Person::get_id() const { return id; }

double Person::get_susceptibility(StrainType strain) const { return susceptibility[strain]; }
void Person::set_susceptibility(StrainType strain, double s) { susceptibility[strain] = s; }

double Person::get_vaccine_protection(StrainType strain) const { return vaccine_protection[strain]; }
void Person::set_vaccine_protection(StrainType strain, double vp) { vaccine_protection[strain] = vp; }

const std::vector<std::unique_ptr<Infection>>& Person::get_infection_history() const { return infection_history; }

Infection* Person::attempt_infection(StrainType strain, size_t time) {
    Infection* inf = nullptr;
    if (is_susceptible_to(strain)) {
        auto current_suscep = susceptibility[strain];
        current_suscep *= is_vaccinated() ? 1 - vaccine_protection[strain] : 1;

        if (rng->draw_from_rng(INFECTION) < current_suscep) {
            auto pr_symptoms    = (strain == INFLUENZA)
                                      ? par->get("pr_sympt_flu")
                                      : par->get("pr_sympt_nonflu");
            auto pr_careseeking = (is_vaccinated())
                                      ? par->get("pr_careseeking_vaxd")
                                      : par->get("pr_careseeking_unvaxd");
            auto sympt = (rng->draw_from_rng(INFECTION) < pr_symptoms) ? SYMPTOMATIC : ASYMPTOMATIC;
            auto seek_care = sympt == SYMPTOMATIC ? rng->draw_from_rng(BEHAVIOR) < pr_careseeking : false;
            infection_history.push_back(std::make_unique<Infection>(this, strain, time, sympt, seek_care));

            inf = infection_history.back().get();
        }
    }

    return inf;
}

bool Person::vaccinate() {
    if (vaccination_status == VACCINATED) { return false; }
    vaccination_status = VACCINATED;
    vaccine_protection = par->sample_vaccine_effect();
    susceptibility = par->sample_susceptibility(this);
    return true;
}

bool Person::has_been_infected() const { return infection_history.size() > 0; }

bool Person::has_been_infected_with(StrainType strain) const {
    return std::any_of(
        infection_history.begin(),
        infection_history.end(),
        [strain](const std::unique_ptr<Infection>& i) { return i->get_strain() == strain; }
    );
}

bool Person::is_vaccinated() const { return vaccination_status == VACCINATED; }

bool Person::is_susceptible_to(StrainType strain) const {
    switch (strain) {
        case INFLUENZA: {
            return not has_been_infected_with(INFLUENZA);
        }
        case NON_INFLUENZA:
            [[fallthrough]];
        default:
            return true;
    }
}

Infection* Person::most_recent_infection() const {
    auto inf = has_been_infected() ? infection_history.back().get() : nullptr;
    return inf;
}

std::ostream& operator<<(std::ostream& o, const Person& p) {
    return o << "Person ID: " << p.id << '\n'
      << "\tsusceptibility (flu, nonflu): " << p.susceptibility[INFLUENZA] << ' ' << p.susceptibility[NON_INFLUENZA] << '\n'
      << "\tvaccination status: " << p.vaccination_status << '\n'
      << "\tvax protection (flu, nonflu): " << p.vaccine_protection[INFLUENZA] << ' ' << p.vaccine_protection[NON_INFLUENZA] << '\n';
}

void Person::update_susceptibility() {}