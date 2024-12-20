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
#include <cmath>

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
    vaccination_time   = par->get("sim_duration") + 1;
    vaccine_protection = std::vector<double>(NUM_STRAIN_TYPES, 0.0);

    susceptibility = par->sample_susceptibility(this);
}

Person::~Person() {}

size_t Person::get_id() const { return id; }

double Person::get_susceptibility(StrainType strain) const { return susceptibility[strain]; }
void Person::set_susceptibility(StrainType strain, double s) { susceptibility[strain] = s; }

double Person::get_current_susceptibility(StrainType strain, size_t time) const {
    bool immunity_generated = (strain == INFLUENZA)
                                  ? par->get("flu_inf_gen_immunity")
                                  : par->get("nonflu_inf_gen_immunity");

    bool immunity_wanes = (strain == INFLUENZA)
                              ? par->get("flu_inf_immunity_wanes")
                              : par->get("nonflu_inf_immunity_wanes");

    if (has_been_infected_with(strain)) {
        if (immunity_generated and immunity_wanes) {
            const auto half_life = (strain == INFLUENZA)
                                       ? par->get("flu_inf_immunity_half_life")
                                       : par->get("nonflu_inf_immunity_half_life");

            const auto waning_rate = util::exp_decay_rate_from_half_life(half_life);

            const auto refactory_period = (strain == INFLUENZA)
                                              ? par->get("flu_inf_refact_len")
                                              : par->get("nonflu_inf_refact_len");

            // this starts waning only after the refactory period length
            // during the refactory period, suscep will be negative though this
            // wont matter because is_susceptible_to() is false during the refactory period
            const auto time_since_last_inf = time - (most_recent_infection(strain)->get_infection_time() + refactory_period);

            // 1 - exp flips the decay and will allow suscep to rise from zero to its original value
            return susceptibility[strain] * (1 - util::exp_decay(waning_rate, time_since_last_inf));
        } else {
            // if immunity is generated and doesnt wane, the individual is perfectly protected forever (suscep = 0)
            // otherwise, when no immunity is generated, they have constant susceptibility
            return (immunity_generated) ? 0 : susceptibility[strain];
        }
    } else {
        // when no immunity is generated, the individual has constant susceptibility
        return susceptibility[strain];
    }
}

double Person::get_vaccine_protection(StrainType strain) const { return vaccine_protection[strain]; }
void Person::set_vaccine_protection(StrainType strain, double vp) { vaccine_protection[strain] = vp; }

double Person::get_remaining_vaccine_protection(StrainType strain, size_t time) const {
    const bool efficacy_wanes = (strain == INFLUENZA)
                                    ? par->get("flu_vax_effect_wanes")
                                    : par->get("nonflu_vax_effect_wanes");

    if (efficacy_wanes) {
        const auto half_life = (strain == INFLUENZA)
                                   ? par->get("flu_vax_effect_half_life")
                                   : par->get("nonflu_vax_effect_half_life");
        const auto waning_rate = util::exp_decay_rate_from_half_life(half_life);
        return vaccine_protection[strain] * util::exp_decay(waning_rate, time);
    } else {
        return vaccine_protection[strain];
    }
}

const std::vector<std::unique_ptr<Infection>>& Person::get_infection_history() const { return infection_history; }

Infection* Person::attempt_infection(StrainType strain, size_t time) {
    Infection* inf = nullptr;
    if (is_susceptible_to(strain, time)) {
        auto current_suscep = get_current_susceptibility(strain, time);
        current_suscep *= is_vaccinated() ? 1 - get_remaining_vaccine_protection(strain, time) : 1;

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

bool Person::vaccinate(size_t time) {
    if (vaccination_status == VACCINATED) { return false; }
    vaccination_status = VACCINATED;
    vaccination_time   = time;
    vaccine_protection = par->sample_vaccine_effect();
    susceptibility     = par->sample_susceptibility(this);
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

bool Person::is_susceptible_to(StrainType strain, size_t time) const {
    // if within any infection refactory period, return false
    const auto last_inf = most_recent_infection();
    if (last_inf != nullptr) {
        const auto time_since_last_inf = time - last_inf->get_infection_time();

        double refactory_period = 0;
        switch (last_inf->get_strain()) {
            case INFLUENZA: { refactory_period = par->get("flu_inf_refact_len"); break; }
            case NON_INFLUENZA: { refactory_period = par->get("nonflu_inf_refact_len"); break; }
            default: { break; }
        }

        if (time_since_last_inf < refactory_period) { return false; }
    }

    // if current suscep <= 0, return false
    if (get_current_susceptibility(strain, time) <= 0) { return false; }

    // otherwise, return true
    return true;
}

Infection* Person::most_recent_infection() const {
    auto inf = has_been_infected() ? infection_history.back().get() : nullptr;
    return inf;
}

Infection* Person::most_recent_infection(StrainType strain) const {
    auto itr = std::find_if(infection_history.crbegin(),
                            infection_history.crend(),
                            [strain](const std::unique_ptr<Infection>& i) { return i->get_strain() == strain; });

    return (itr != infection_history.crend()) ? itr->get() : nullptr;
}

size_t Person::last_infection_time() const { return most_recent_infection()->get_infection_time(); }
size_t Person::last_infection_strain() const { return most_recent_infection()->get_strain(); }

std::ostream& operator<<(std::ostream& o, const Person& p) {
    return o << "Person ID: " << p.id << '\n'
      << "\tsusceptibility (flu, nonflu): " << p.susceptibility[INFLUENZA] << ' ' << p.susceptibility[NON_INFLUENZA] << '\n'
      << "\tvaccination status: " << p.vaccination_status << '\n'
      << "\tvax protection (flu, nonflu): " << p.vaccine_protection[INFLUENZA] << ' ' << p.vaccine_protection[NON_INFLUENZA] << '\n';
}

void Person::update_susceptibility() {}