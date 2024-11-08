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
    Tome(sol::state* lua_vm, std::string core_config);
    ~Tome();

    std::map<std::string, sol::object> get_config_core() const;
    std::map<std::string, sol::object> get_config_params() const;
    std::map<std::string, sol::object> get_config_metrics() const;

    sol::object get_element(std::string key) const;

    template<typename T = double>
    T get_element_as(std::string key) const {
        return get_element(key).as<T>();
    }

    std::string database_path() const;

    void clean();

  private:
    bool check_for_req_items(sol::table core_tome_table);
    void slurp_table(sol::table from, std::map<std::string, sol::object>& into);

    std::map<std::string, sol::object> config_core;
    std::map<std::string, sol::object> config_params;
    std::map<std::string, sol::object> config_metrics;

    std::map<std::string, std::map<std::string, sol::object>*> element_lookup;

    const std::string core_config_path;

    sol::state* vm;
};