/**
 * @file tome.cpp
 * @author Alexander N. Pillai
 * @brief Contains the Tome class that is responsible for processing and storing
 *        all user configuration data.
 *
 * @copyright TBD
 */
#include <map>
#include <string>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <storyteller/tome.hpp>

Tome::Tome(sol::state* lua_vm, std::string core_config)
    : core_config_path(core_config),
      vm(lua_vm) {
    vm->open_libraries(sol::lib::base);
    vm->script_file(core_config_path);

    // check for tome object in the core config
    sol::optional<sol::table> core_tome_table = (*vm)["tome"];
    if (core_tome_table == sol::nullopt) {
        std::cerr << "ERROR: core config file must include a `tome` table.\n";
        exit(-1);
    }

    auto sane_config = check_for_req_items(core_tome_table.value());
    if (not sane_config) {
        std::cerr << "ERROR: core config file missing required element.\n";
        exit(-1);
    }

    slurp_table(core_tome_table.value(), config_core);

    vm->script_file(config_core["parameters"].as<std::string>());
    sol::optional<sol::table> param_table = (*vm)["parameters"];
    if (param_table == sol::nullopt) {
        std::cerr << "ERROR: parameter config file must include a `parameters` table.\n";
        exit(-1);
    }

    slurp_table(param_table.value(), config_params);

    vm->script_file(config_core["metrics"].as<std::string>());
    sol::optional<sol::table> metrics_table = (*vm)["metrics"];
    if (metrics_table == sol::nullopt) {
        std::cerr << "ERROR: parameter config file must include a `metrics` table.\n";
        exit(-1);
    }

    slurp_table(metrics_table.value(), config_metrics);
}

Tome::~Tome() {}

std::map<std::string, sol::object> Tome::get_config_core()    const { return config_core; }
std::map<std::string, sol::object> Tome::get_config_params()  const { return config_params; }
std::map<std::string, sol::object> Tome::get_config_metrics() const { return config_metrics; }

sol::object Tome::get_element(std::string key) const {
    auto dict = element_lookup.at(key);
    return dict->at(key);
}

std::string Tome::database_path() const {
    bool user_defined_db_path = element_lookup.count("database_path");
    std::string db_path = user_defined_db_path
                              ? get_element_as<std::string>("database_path")
                              : get_element_as<std::string>("experiment_name") + ".sqlite";
    return db_path;
}

bool Tome::check_for_req_items(sol::table core_tome_table) {
    sol::optional<std::string> exp_name = core_tome_table["experiment_name"];
    sol::optional<size_t> n_rlztns      = core_tome_table["n_realizations"];
    sol::optional<size_t> sim_dur       = core_tome_table["sim_duration"];
    sol::optional<std::string> par_file = core_tome_table["parameters"];
    sol::optional<std::string> met_file = core_tome_table["metrics"];

    return exp_name and n_rlztns and sim_dur and par_file and met_file;
}

void Tome::slurp_table(sol::table from, std::map<std::string, sol::object>& into) {
    for (const auto& el : from) {
        auto el_key = el.first.as<std::string>();
        element_lookup[el_key] = &into;
        into[el_key] = el.second;
    }
}

void Tome::clean() {
    config_core = {};
    config_params = {};
    config_metrics = {};
    element_lookup = {};
    vm->collect_garbage();
}