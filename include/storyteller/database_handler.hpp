/**
 * @file database_handler.hpp
 * @author Alexander N. Pillai
 * @brief Contains the DatabaseHandler class that performs all SQLite database
 *        operations.
 *
 * @copyright TBD
 */
#pragma once

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Storyteller;
class Ledger;
class Parameters;

enum TableName {
    PAR,
    MET,
    JOB,
    NUM_TABLE_NAMES
};

enum ConfigParFlag {
    CONST,
    COPY,
    STEP,
    NUM_CONFIG_PAR_FLAGS
};

/**
 * @brief Handles all SQLite database operations.
 * 
 * Includes methods that create a new experiment database using the user-provided
 * configuration file, read simulation parameters for a specific particle, and
 * write simulation metrics to the database after a simulation terminates.
 */
class DatabaseHandler {
  public:
    DatabaseHandler(const Storyteller* storyteller, std::string db_path);
    ~DatabaseHandler();

    int init_database(json cfg);
    void create_table();
    void clear_table();

    bool database_exists();
    bool table_exists(std::string table);

    void read_parameters(unsigned int serial, std::map<std::string, double>& pars);
    void write_metrics(const Ledger* ledger, const Parameters* par) const;
    void start_job(unsigned int serial);

  private:
    std::vector<std::string> prepare_insert_sql(const Ledger* ledger, const Parameters* par) const;

    std::string database_path;
    size_t n_transaction_attempts;
    size_t ms_delay_between_attempts;

    // const Storyteller* owner;
};