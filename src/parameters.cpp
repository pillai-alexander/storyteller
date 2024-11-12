/**
 * @file parameters.cpp
 * @author Alexander N. Pillai
 * @brief Contains the Parameters class that stores all necessary simulaiton
 *        parameters and contains helper functions that use the parameter values.
 *
 * @copyright TBD
 */
#include <algorithm>
#include <vector>
#include <array>
#include <numeric>
#include <string>
#include <memory>
#include <limits>
#include <cmath>

#include <gsl/gsl_randist.h>
#include <sol/sol.hpp>

#include <storyteller/parameters.hpp>
#include <storyteller/person.hpp>
#include <storyteller/simulator.hpp>
#include <storyteller/utility.hpp>
#include <storyteller/database_handler.hpp>
#include <storyteller/tome.hpp>

Parameter::Parameter(const std::string name, const sol::table& attributes)
    : fullname(name),
      nickname(attributes.get<std::string>("nickname")),
      description(attributes.get<std::string>("description")),
      flag(attributes.get<std::string>("flag")),
      datatype(attributes.get<std::string>("datatype")),
      _validate(attributes.get<sol::function>("validate")) {
    value = (flag == "const") ? attributes.get<double>("value") : std::numeric_limits<double>::infinity();
}

inline std::string Parameter::get_fullname() const { return fullname; }
inline std::string Parameter::get_nickname() const { return nickname; }
inline double      Parameter::get_value()    const { return value; }
inline bool        Parameter::validate()     const { return _validate(value); }

Parameters::Parameters(RngHandler* rngh, DatabaseHandler* dbh, const Tome* t)
    : rng(rngh),
      db(dbh),
      tome(t) {
    database_path = tome->get_path("database");

    return_metrics.clear();
    for (auto& [key, el] : tome->get_config_metrics()) {
        return_metrics.push_back(key);
    }

    sol::optional<sol::table> pars = tome->get_config_params().at("parameters").as<sol::table>();
    if (pars) {
        for (const auto& [key, obj] : pars.value()) {
            auto fullname   = key.as<std::string>();
            auto attributes = obj.as<sol::table>();
            auto nickname   = attributes.get<std::string>("nickname");
            auto flag       = attributes.get<std::string>("flag");
            if (flag != "const") pars_to_read.insert({nickname, std::numeric_limits<double>::infinity()});
            insert(fullname, attributes);
        }
        calc_strain_probs();
    }
}

Parameter::~Parameter() { _validate = {}; }

void Parameters::read_parameters_for_serial(size_t serial) {
    simulation_serial = serial;
    db->read_parameters(serial, pars_to_read);

    bool can_continue = false;
    for (const auto& [k, v] : pars_to_read) {
        if (std::isinf(v)) {
            can_continue = false;
            std::cerr << "ERROR: " << k << " not found\n";
        } else {
            can_continue = true;
        }
    }

    if (can_continue) {
        rng->set_seed(pars_to_read.at("seed"));
        pars_to_read.erase("seed");

        for (const auto& [nickname, value] : pars_to_read) {
            auto fullname = lookup.at(nickname);
            params.at(fullname)->value = value;
        }
        pars_to_read.clear();
    }

    if (pars_to_read.size() != 0) {
        std::cerr << "ERROR: some params not found\n";
        exit(-1);
    }

    calc_strain_probs();
}

bool Parameters::insert(const std::string key, const sol::table& attributes) {
    auto nickname = attributes.get<std::string>("nickname");
    lookup[key] = key;
    lookup[nickname] = key;
    auto ret = params.insert({key, std::make_unique<Parameter>(key, attributes)});
    return ret.second;
}

double Parameters::get(std::string key) const {
    auto fullname = lookup.at(key);
    auto p = params.at(fullname).get();
    return p->get_value();
}

void Parameters::calc_strain_probs() {
    strain_probs = std::vector<double>(NUM_STRAIN_TYPES + 1, 0.0);

    strain_probs[NON_INFLUENZA] = get("pr_nonflu_exposure");
    strain_probs[INFLUENZA]     = get("pr_flu_exposure");
    strain_probs[NUM_STRAIN_TYPES] = 1.0 - (strain_probs[NON_INFLUENZA] + strain_probs[INFLUENZA]);
}

double Parameters::sample_discrete_susceptibility(const bool vaccinated, const double suscep_w_prior, const double suscep_wo_prior) const {
    auto pr_prior_immunity = (vaccinated) ? get("pr_prior_imm_vaxd") : get("pr_prior_imm_unvaxd");
    if (pr_prior_immunity == 0.0) {
        return suscep_wo_prior;
    } else {
        return (rng->draw_from_rng(INFECTION) < pr_prior_immunity) ? suscep_w_prior : suscep_wo_prior;
    }
}

double Parameters::sample_continuous_susceptibility(const double shape, const double mean) const {
    auto scale = util::gamma_scale_from_mean(shape, mean);
    return gsl_ran_gamma(rng->get_rng(INFECTION), shape, scale);
}

std::vector<double> Parameters::sample_susceptibility(const Person* p) const {
    std::vector<double> susceps(NUM_STRAIN_TYPES, 1.0);

    auto is_vaxd = p->is_vaccinated();

    auto flu_shape    = (is_vaxd) ? get("vaxd_flu_suscep_shape")    : get("unvaxd_flu_suscep_shape");
    auto flu_mean     = (is_vaxd) ? get("vaxd_flu_suscep_mean")     : get("unvaxd_flu_suscep_mean");
    auto flu_baseline = (is_vaxd) ? get("vaxd_flu_suscep_baseline") : get("unvaxd_flu_suscep_baseline");

    auto nonflu_shape    = (is_vaxd) ? get("vaxd_nonflu_suscep_shape")    : get("unvaxd_nonflu_suscep_shape");
    auto nonflu_mean     = (is_vaxd) ? get("vaxd_nonflu_suscep_mean")     : get("unvaxd_nonflu_suscep_mean");
    auto nonflu_baseline = (is_vaxd) ? get("vaxd_nonflu_suscep_baseline") : get("unvaxd_nonflu_suscep_baseline");

    susceps[NON_INFLUENZA] = (nonflu_shape == 0.0)
                                 ? sample_discrete_susceptibility(is_vaxd, nonflu_mean, nonflu_baseline)
                                 : sample_continuous_susceptibility(nonflu_shape, nonflu_mean);
    susceps[INFLUENZA]     = (flu_shape == 0.0)
                                 ? sample_discrete_susceptibility(is_vaxd, flu_mean, flu_baseline)
                                 : sample_continuous_susceptibility(flu_shape, flu_mean);
    return susceps;
}

double Parameters::sample_discrete_vaccine_effect(const double b) const {
    return b;
}

double Parameters::sample_continuous_vaccine_effect(const double a, const double b) const {
    return gsl_ran_beta(rng->get_rng(VACCINATION), a, b);
}

std::vector<double> Parameters::sample_vaccine_effect() const {
    std::vector<double> vax_effects(NUM_STRAIN_TYPES, 0.0);

    auto flu_a    = get("flu_vax_effect_a");
    auto flu_b    = get("flu_vax_effect_b");
    auto nonflu_a = get("nonflu_vax_effect_a");
    auto nonflu_b = get("nonflu_vax_effect_b");

    vax_effects[NON_INFLUENZA] = (nonflu_a == 0.0)
                                     ? sample_discrete_vaccine_effect(nonflu_b)
                                     : sample_continuous_vaccine_effect(nonflu_a, nonflu_b);
    vax_effects[INFLUENZA]     = (nonflu_a == 0.0)
                                     ? sample_discrete_vaccine_effect(flu_b)
                                     : sample_continuous_vaccine_effect(flu_a, flu_b);
    return vax_effects;
}

StrainType Parameters::sample_strain() const {
    const size_t num_categories = NUM_STRAIN_TYPES + 1;
    std::vector<unsigned int> sample(num_categories, 0);
    gsl_ran_multinomial(
        rng->get_rng(INFECTION),
        num_categories,
        constants::ONE,
        strain_probs.data(),
        sample.data()
    );

    auto idx = std::find(sample.begin(), sample.end(), constants::ONE) - sample.begin();
    return (StrainType) idx;
}

bool Parameters::are_valid() const {
    std::vector<bool> rets;
    for (const auto& [fullname, p] : params) {
        bool p_is_valid = p->validate();
        if (not p_is_valid) {
            rets.push_back(false);
            std::cerr << "ERROR: " << fullname <<  " has invalid value = " << p->value << '\n';
        } else {
            rets.push_back(true);
        }
    }
    return std::all_of(rets.cbegin(), rets.cend(), [](bool v){ return (v == true);});
}

// void Parameters::update_time_varying_parameters() {}