/**
 * @file parameters.hpp
 * @author Alexander N. Pillai
 * @brief Contains the Parameters class that stores all necessary simulaiton
 *        parameters and contains helper functions that use the parameter values.
 *
 * @copyright TBD
 */
#pragma once

#include <vector>
#include <array>
#include <string>
#include <map>
#include <memory>
#include <iostream>

#include <sol/sol.hpp>

class Person;
class RngHandler;
class DatabaseHandler;
class Tome;

enum StrainType {
    NON_INFLUENZA,
    INFLUENZA,
    NUM_STRAIN_TYPES
};

enum VaccinationStatus {
    UNVACCINATED,
    VACCINATED,
    NUM_VACCINATION_STATUSES
};

enum SymptomClass {
    ASYMPTOMATIC,
    SYMPTOMATIC,
    NUM_SYMPTOM_CLASSES
};

enum BetaDistributionParameter {
    A,
    B,
    NUM_BETA_DISTR_PARAMS
};

typedef std::array<double, NUM_BETA_DISTR_PARAMS> BetaDistrParamArray;

enum GammaDistributionParameter {
    SHAPE,
    SCALE,
    NUM_GAMMA_DISTR_PARAMS
};

typedef std::array<double, NUM_GAMMA_DISTR_PARAMS> GammaDistrParamArray;

class Parameter {
  friend class Parameters;
  public:
    Parameter(const std::string name, const sol::table& attributes);
    ~Parameter();

    std::string get_fullname() const;
    std::string get_nickname() const;
    double      get_value()    const;

    bool validate() const;

  private:
    std::string    fullname;
    std::string    nickname;
    std::string    description;
    std::string    flag;
    std::string    datatype;
    double         value;
    sol::protected_function _validate;
};

/**
 * @brief Stores all necessary parameters to perform a single simulation.
 *
 * A Parameters object can either be default constructed and use pre-defined
 * parameter values or can be initialized using parameter values read by the
 * Storyteller from the experiment database.
 */
class Parameters {
  public:
    Parameters(RngHandler* rngh, DatabaseHandler* dbh, const Tome* t);
    ~Parameters() = default;

    void read_parameters_for_serial(size_t serial);
    void read_parameters_from_batch(size_t serial, std::map<std::string, double> pars_from_db);

    bool insert(const std::string key, const sol::table& attributes);
    double get(std::string key) const;

    std::vector<double> sample_susceptibility(const Person* p) const;
    std::vector<double> sample_vaccine_effect() const;
    StrainType sample_strain() const;
    std::vector<StrainType> daily_strain_sample() const;

    bool are_valid() const;

    // void update_time_varying_parameters();

    std::vector<double> strain_probs;

    std::string linelist_file_path;
    std::string simvis_file_path;
    std::string database_path;
    size_t simulation_serial;

    std::vector<std::string> return_metrics;

    const Tome* tome;

  private:
    std::map<std::string, std::unique_ptr<Parameter>> params;
    std::map<std::string, std::string> lookup;

    std::vector<std::string> pars_to_read;

    void calc_strain_probs();
    void slurp_params(std::map<std::string, double> pars_from_db);

    double sample_discrete_susceptibility(const bool vaccinated, const StrainType strain) const;
    double sample_continuous_susceptibility(const bool vaccinated, const StrainType strain) const;

    double sample_discrete_vaccine_effect(const StrainType strain) const;
    double sample_continuous_vaccine_effect(const StrainType strain) const;

    RngHandler* rng;
    DatabaseHandler* db;
};