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

class Storyteller;
class Ledger;
class Parameters;
class Tome;

enum TableName {
    PAR,
    MET,
    JOB,
    NUM_TABLE_NAMES
};

// enum ConfigParFlag {
//     CONST,
//     // COPY,
//     STEP,
//     NUM_CONFIG_PAR_FLAGS
// };

class ParticleJob {
  public:
    ParticleJob();
    ParticleJob(size_t serial);

    void start();
    void end();

    std::string update();

    size_t serial;
    size_t attempts;
    size_t completions;
    std::string status;
    int start_time;
    int end_time;
    int duration;
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
    DatabaseHandler(const Storyteller* storyteller);
    ~DatabaseHandler();

    int init_database();

    bool database_exists();
    bool table_exists(std::string table);

    std::map<std::string, double> read_parameters(unsigned int serial, const std::vector<std::string>& pars);
    std::vector<std::map<std::string, double>> read_batch_parameters(unsigned int serial_start, unsigned int serial_end, const std::vector<std::string>& pars);
    void write_metrics(const Ledger* ledger, const Parameters* par);

    void start_job(unsigned int serial);
    void end_job(unsigned int serial);
    void end_jobs(std::vector<ParticleJob>& jobs);

    void drop_table_if_exists(std::string table);
    void import_metrics_from(std::string file_path);

  private:
    void create_table();
    void clear_table();

    void read_job(unsigned int serial);

    void clear_metrics(unsigned int serial);

    std::vector<std::string> prepare_insert_sql(const Ledger* ledger, const Parameters* par) const;

    std::string database_path;
    size_t n_transaction_attempts;
    size_t ms_delay_between_attempts;

    const Storyteller* owner;
    const Tome* tome;
    ParticleJob simulation_job;
};