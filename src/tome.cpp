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
#include <filesystem>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <storyteller/tome.hpp>

namespace fs = std::filesystem;

Tome::Tome(sol::state* lua_vm, std::string path)
    : vm(lua_vm),
      tome_path(path) {
    vm->open_libraries();
    vm->script_file(tome_path);

    // check for tome object in the core config
    sol::optional<sol::table> core_tome_table = vm->get<sol::table>("Tome");
    if (core_tome_table == sol::nullopt) {
        std::cerr << "ERROR: core config file must include a `Tome` table.\n";
        exit(-1);
    }

    auto sane_config = check_for_req_items(core_tome_table.value());
    if (not sane_config) {
        std::cerr << "ERROR: core config file missing required element.\n";
        exit(-1);
    }

    slurp_table(core_tome_table.value(), config_core);

    fs::path parameters_config_path = tome_path.parent_path();
    parameters_config_path /= config_core["parameters"].as<std::string>();

    vm->script_file(parameters_config_path);
    sol::optional<sol::table> param_table = vm->get<sol::table>("Parameters");
    if (param_table == sol::nullopt) {
        std::cerr << "ERROR: parameter config file must include a `Parameters` table.\n";
        exit(-1);
    }

    slurp_table(param_table.value(), config_params);

    fs::path metrics_config_path = tome_path.parent_path();
    metrics_config_path /= config_core["metrics"].as<std::string>();

    vm->script_file(metrics_config_path);
    sol::optional<sol::table> metrics_table = vm->get<sol::table>("Metrics");
    if (metrics_table == sol::nullopt) {
        std::cerr << "ERROR: parameter config file must include a `Metrics` table.\n";
        exit(-1);
    }

    slurp_table(metrics_table.value(), config_metrics);

    determine_paths();
}

std::map<std::string, sol::object> Tome::get_config_core()    const { return config_core; }
std::map<std::string, sol::object> Tome::get_config_params()  const { return config_params; }
std::map<std::string, sol::object> Tome::get_config_metrics() const { return config_metrics; }

sol::object Tome::get_element(std::string key) const {
    auto dict = element_lookup.at(key);
    return dict->at(key);
}

std::string Tome::get_path(std::string key) const { return paths.at(key); }

void Tome::determine_paths() {
    fs::path tome_root = tome_path.is_absolute()
                             ? tome_path.parent_path()
                             : fs::current_path() / tome_path.parent_path();

    fs::path storyteller_root = fs::path(__FILE__).parent_path().parent_path();

    bool user_defined_db_path = element_lookup.count("database_path");
    fs::path db_path = user_defined_db_path
                   ? get_element_as<std::string>("database_path")
                   : get_element_as<std::string>("experiment_name") + ".sqlite";

    paths["tome_rt"]  = tome_root;
    paths["database"] = tome_root / db_path;
    paths["simvis"]   = tome_root / "simvis.out";
    paths["synthpop"] = tome_root / "synthpop.out";
    paths["linelist"] = tome_root / "linelist.out";
    paths["scripts"]  = storyteller_root / "scripts";
    paths["simvis.R"] = paths.at("scripts") / "simvis.R";
}

bool Tome::check_for_req_items(sol::table core_tome_table) {
    sol::optional<std::string> exp_name = core_tome_table["experiment_name"];
    sol::optional<size_t> n_rlztns      = core_tome_table["n_realizations"];
    sol::optional<std::string> par_file = core_tome_table["parameters"];
    sol::optional<std::string> met_file = core_tome_table["metrics"];

    return exp_name and n_rlztns and par_file and met_file;
}

void Tome::slurp_table(sol::table& from, std::map<std::string, sol::object>& into) {
    for (const auto& [name, obj] : from) {
        auto key = name.as<std::string>();
        element_lookup[key] = &into;
        into[key] = obj;
    }
}

void Tome::clean() {
    config_core.clear();
    config_params.clear();
    config_metrics.clear();
    element_lookup.clear();
    vm->collect_garbage();
}