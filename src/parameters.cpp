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
      _validate(attributes.get<sol::function>("validate")),
      value(std::numeric_limits<double>::infinity()) {}

Parameter::~Parameter() { _validate = {}; }

inline std::string Parameter::get_fullname() const { return fullname; }
inline std::string Parameter::get_nickname() const { return nickname; }
inline double      Parameter::get_value()    const { return value; }
inline bool        Parameter::validate()     const { return _validate(value); }

Parameters::Parameters(RngHandler* rngh, DatabaseHandler* dbh, const Tome* t)
    : rng(rngh),
      db(dbh),
      tome(t),
      pars_to_read({"seed"}) {
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
            auto nickname   = attributes.get_or<std::string>("nickname", fullname);

            pars_to_read.push_back(nickname);
            insert(fullname, attributes);
        }
    }
}

void Parameters::read_parameters_for_serial(size_t serial) {
    simulation_serial = serial;
    auto pars_from_db = db->read_parameters(serial, pars_to_read);

    if (pars_to_read.size() == pars_from_db.size()) {
        for (const auto& nickname : pars_to_read) {
            if (nickname == "seed") {
                rng->set_seed(pars_from_db.at("seed"));
                pars_from_db.erase("seed");
            } else {
                auto fullname = lookup.at(nickname);
                params.at(fullname)->value = pars_from_db.at(nickname);
            }
        }
    } else {
        std::cerr << "ERROR: number of params read (" << pars_from_db.size()
                  << ") does not match the number expected (" << pars_to_read.size() << ").\n";
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

double Parameters::sample_discrete_susceptibility(const bool vaccinated, const StrainType strain) const {
    auto suscep_w_prior = -1.0;
    auto suscep_wo_prior = -1.0;
    switch (strain) {
        case NON_INFLUENZA: {
            suscep_w_prior  = (vaccinated) ? get("vaxd_nonflu_suscep_mean")     : get("unvaxd_nonflu_suscep_mean");
            suscep_wo_prior = (vaccinated) ? get("vaxd_nonflu_suscep_baseline") : get("unvaxd_nonflu_suscep_baseline");
            break;
        }
        case INFLUENZA: {
            suscep_w_prior  = (vaccinated) ? get("vaxd_flu_suscep_mean")     : get("unvaxd_flu_suscep_mean");
            suscep_wo_prior = (vaccinated) ? get("vaxd_flu_suscep_baseline") : get("unvaxd_flu_suscep_baseline");
            break;
        }
        default: {
            std::cerr << "ERROR: unknown strain " << strain << '\n';
            exit(-1);
        }
    }

    if ((suscep_w_prior == -1.0) or (suscep_wo_prior == -1.0)) {
        std::cerr << "ERROR: invalid suscep values\n";
        exit(-1);
    }

    const auto pr_prior_immunity = (vaccinated) ? get("pr_prior_imm_vaxd") : get("pr_prior_imm_unvaxd");
    if (pr_prior_immunity == 0.0) {
        return suscep_wo_prior;
    } else {
        return (rng->draw_from_rng(INFECTION) < pr_prior_immunity) ? suscep_w_prior : suscep_wo_prior;
    }
}

double Parameters::sample_continuous_susceptibility(const bool vaccinated, const StrainType strain) const {
    auto mean = -1.0;
    auto sd = -1.0;
    switch (strain) {
        case NON_INFLUENZA: {
            mean = (vaccinated) ? get("vaxd_nonflu_suscep_mean") : get("unvaxd_nonflu_suscep_mean");
            sd  = (vaccinated) ? get("vaxd_nonflu_suscep_sd")  : get("unvaxd_nonflu_suscep_sd");
            break;
        }
        case INFLUENZA: {
            mean = (vaccinated) ? get("vaxd_flu_suscep_mean") : get("unvaxd_flu_suscep_mean");
            sd  = (vaccinated) ? get("vaxd_flu_suscep_sd")  : get("unvaxd_flu_suscep_sd");
            break;
        }
        default: {
            std::cerr << "ERROR: unknown strain " << strain << '\n';
            exit(-1);
        }
    }

    if ((mean == -1.0) or (sd == -1.0)) {
        std::cerr << "ERROR: invalid values\n";
        exit(-1);
    }

    // gsl gaussian is always centered at 0 and must be shifted to the specified mean
    // https://www.gnu.org/software/gsl/doc/html/randist.html#the-gaussian-distribution
    auto log_odds = util::logit(mean) + gsl_ran_gaussian(rng->get_rng(INFECTION), sd);
    return util::logistic(log_odds);
}

std::vector<double> Parameters::sample_susceptibility(const Person* p) const {
    std::vector<double> susceps(NUM_STRAIN_TYPES, 1.0);

    auto is_vaxd = p->is_vaccinated();

    auto contin_flu_suscep = (is_vaxd)
                                 ? get("vaxd_flu_suscep_is_contin")
                                 : get("unvaxd_flu_suscep_is_contin");
    susceps[INFLUENZA] = (contin_flu_suscep == 0.0)
                             ? sample_discrete_susceptibility(is_vaxd, INFLUENZA)
                             : sample_continuous_susceptibility(is_vaxd, INFLUENZA);

    auto contin_nonflu_suscep = (is_vaxd)
                                    ? get("vaxd_nonflu_suscep_is_contin")
                                    : get("unvaxd_nonflu_suscep_is_contin");
    susceps[NON_INFLUENZA] = (contin_nonflu_suscep == 0.0)
                                 ? sample_discrete_susceptibility(is_vaxd, NON_INFLUENZA)
                                 : sample_continuous_susceptibility(is_vaxd, NON_INFLUENZA);

    return susceps;
}

double Parameters::sample_discrete_vaccine_effect(const StrainType strain) const {
    auto mean = -1.0;
    switch (strain) {
        case NON_INFLUENZA: {
            mean = get("nonflu_vax_effect_mean");
            break;
        }
        case INFLUENZA: {
            mean = mean = get("flu_vax_effect_mean");
            break;
        }
        default: {
            std::cerr << "ERROR: unknown strain " << strain << '\n';
            exit(-1);
        }
    }

    if (mean == -1.0) {
        std::cerr << "ERROR: invalid values\n";
        exit(-1);
    }

    return mean;
}

double Parameters::sample_continuous_vaccine_effect(const StrainType strain) const {
    auto mean = -1.0;
    auto var = -1.0;
    switch (strain) {
        case NON_INFLUENZA: {
            mean = get("nonflu_vax_effect_mean");
            var  = get("nonflu_vax_effect_var");
            break;
        }
        case INFLUENZA: {
            mean = get("flu_vax_effect_mean");
            var  = get("flu_vax_effect_var");
            break;
        }
        default: {
            std::cerr << "ERROR: unknown strain " << strain << '\n';
            exit(-1);
        }
    }

    if ((mean == -1.0) or (var == -1.0)) {
        std::cerr << "ERROR: invalid values\n";
        exit(-1);
    }

    const auto a = util::beta_a_from_mean_var(mean, var);
    const auto b = util::beta_b_from_mean_var(mean, var);
    return gsl_ran_beta(rng->get_rng(VACCINATION), a, b);
}

std::vector<double> Parameters::sample_vaccine_effect() const {
    std::vector<double> vax_effects(NUM_STRAIN_TYPES, 0.0);

    const auto contin_flu_vax = get("flu_vax_effect_is_contin");
    vax_effects[INFLUENZA] = (contin_flu_vax == 0.0)
                                 ? sample_discrete_vaccine_effect(INFLUENZA)
                                 : sample_continuous_vaccine_effect(INFLUENZA);

    const auto contin_nonflu_vax = get("nonflu_vax_effect_is_contin");
    vax_effects[NON_INFLUENZA] = (contin_nonflu_vax == 0.0)
                                     ? sample_discrete_vaccine_effect(NON_INFLUENZA)
                                     : sample_continuous_vaccine_effect(NON_INFLUENZA);

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