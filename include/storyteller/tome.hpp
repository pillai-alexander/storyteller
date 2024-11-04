/**
 * @file tome.hpp
 * @author Alexander N. Pillai
 * @brief Contains the Tome class that is responsible for processing and storing
 *        all user configuration data.
 *
 * @copyright TBD
 */
#pragma once

#include <map>
#include <string>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

class Tome {
  public:
    Tome(std::string core_config);
    ~Tome();
  private:
    bool check_for_req_items(sol::table core_tome_table);
    std::map<std::string, sol::object> slurp_table(sol::table t);

    std::map<std::string, sol::object> config_core;
    std::map<std::string, sol::object> config_params;
    std::map<std::string, sol::object> config_metrics;

    const std::string core_config_path;

    sol::state vm;
};