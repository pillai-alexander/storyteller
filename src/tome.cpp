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

#include <sol/sol.hpp>

#include <storyteller/tome.hpp>

Tome::Tome(std::string core_config) : core_config_path(core_config) {
    vm.open_libraries(sol::lib::base);
    vm.script_file(core_config_path);

    // check for tome object in the core config
    sol::optional<sol::table> core_tome_table = vm["tome"];
    if (core_tome_table == sol::nullopt) {
        std::cerr << "ERROR: core config file must include a `tome` table.\n";
        exit(-1);
    }

    auto sane_config = check_for_req_items(core_tome_table.value());
    if (not sane_config) {
        std::cerr << "ERROR: core config file missing required element.\n";
        exit(-1);
    }

    config_core = slurp_table(core_tome_table.value());

    vm.script_file(config_core["parameters"].as<std::string>());
    sol::optional<sol::table> param_table = vm["parameters"];
    if (param_table == sol::nullopt) {
        std::cerr << "ERROR: parameter config file must include a `parameters` table.\n";
        exit(-1);
    }

    config_params = slurp_table(param_table.value());

    vm.script_file(config_core["metrics"].as<std::string>());
    sol::optional<sol::table> metrics_table = vm["metrics"];
    if (metrics_table == sol::nullopt) {
        std::cerr << "ERROR: parameter config file must include a `metrics` table.\n";
        exit(-1);
    }

    config_params = slurp_table(metrics_table.value());
}

Tome::~Tome() {}

bool Tome::check_for_req_items(sol::table core_tome_table) {
    sol::optional<std::string> exp_name = core_tome_table["experiment_name"];
    sol::optional<size_t> n_rlztns      = core_tome_table["n_realizations"];
    sol::optional<size_t> sim_dur       = core_tome_table["sim_duration"];
    sol::optional<std::string> par_file = core_tome_table["parameters"];
    sol::optional<std::string> met_file = core_tome_table["metrics"];

    return exp_name and n_rlztns and sim_dur and par_file and met_file;
}

std::map<std::string, sol::object> Tome::slurp_table(sol::table t) {
    std::map<std::string, sol::object> ret;
    for (const auto& el : t) {
        ret[el.first.as<std::string>()] = el.second;
    }
    return ret;
}