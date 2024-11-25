/**
 * @file storyteller.hpp
 * @author Alexander N. Pillai
 * @brief Contains the Storyteller class that is responsible for taking in user
 *        input and performing the desired operations (eg, database initialization,
 *        simulations)
 *
 * @copyright TBD
 */
#pragma once

#include <map>
#include <string>
#include <memory>

#include <argh.h>

class Simulator;
class DatabaseHandler;
class RngHandler;
class Parameters;
class Tome;
namespace sol { class state; }

/**
 * @brief Defines the types of operations that the Storyteller is capable of
 *        performing.
 */
enum OperationType {
    INITIALIZE,
    GENERATE_SYNTHETIC_POPULATION,
    BATCH_SIM,
    SLURP_CSVS_INTO_DATABASE,
    NUM_OPERATION_TYPES
};

/**
 * @brief Primary object that interfaces with the user, processes user input,
 *        and performs desired operations.
 *
 * A Storyteller object is created by the user's main file and handles all user
 * input (including command-line argumets and configuration files). The object
 * then processes the user input to determine what operation is desired (currently
 * either database construction or simulation) and performs that operation.
 */
class Storyteller {
  public:
    /**
     * @brief Construct a new Storyteller object using all user-provided input.
     *
     * @param argc Number of command-line arguments
     * @param argv Array of command-line arguments
     */
    Storyteller(int argc, char* argv[]);
    ~Storyteller();

    const Parameters* get_parameters() const;
    const Tome* get_tome() const;

    /**
     * @brief Get the simulation serial.
     *
     * @return int Simulation serial number
     */
    int get_serial() const;

    /**
     * @brief Get the number of simulations to be run in the batch.
     *
     * @return size_t Simulation batch size
     */
    size_t get_batch_size() const;

    /**
     * @brief Get the path to the configuration file.
     *
     * @return std::string Configuration file path
     */
    std::string get_config_file() const;

    /**
     * @brief Get a specific program flag.
     *
     * @param key Name of the desired program flag as a string.
     * @return true Flag "key" is set to true
     * @return false Flag "key" is set to false
     */
    bool get_flag(std::string key) const;

    /**
     * @brief Set a specific program flag.
     *
     * @param key Name of the desired flag as a string
     * @param val Boolean value for flag "key"
     */
    void set_flag(std::string key, bool val);

    /**
     * @brief Run the user-desired operation.
     *
     * @return int Return code of the operation (0 if sucessful)
     */
    int run();

  private:
    /**
     * @todo: process user input to determine if it matches expectations
     *
     * @return true
     * @return false
     */
    bool sensible_inputs() const;

    /**
     * @brief Initialize Storteller for running a batch of simulations.
     */
    void init_batch();

    /**
     * @brief Initialize Storteller for running a single simulation job.
     */
    void init_simulation();

    /**
     * @brief Constructs a new experiment database given the user-provided
     *        configuration file.
     *
     * @return int Return code (0 if sucessful)
     */
    int construct_database();

    /**
     * @brief Runs a batch of simulations drawn from an experiment database.
     *
     * @return int Return code (0 if sucessful)
     */
    int batch_simulation();

    /**
     * @brief Calls the Rscript to generate the simulation dashboard.
     *
     * @return int Return code (0 if sucessful)
     */
    int draw_simvis();

    /**
     * @brief Resets the Storteller before a new simulation.
     */
    void reset();

    int generate_synthpop();

    int slurp_metrics_files();

    std::unique_ptr<Tome> tome;
    std::unique_ptr<Simulator> simulator;           ///< Created for each simulation to be run
    std::unique_ptr<DatabaseHandler> db_handler;    ///< Handles all database operations
    std::unique_ptr<RngHandler> rng_handler;        ///< Handles all pseudo-random number generation
    std::unique_ptr<Parameters> parameters;         ///< Stores all necessary simulation parameters
    std::unique_ptr<sol::state> lua_vm;

    OperationType operation_to_perform;

    argh::parser cmdl_args;                         ///< Stores parsed command-line arguments
    std::map<std::string, bool> simulation_flags;   ///< Stores program flags

    int simulation_serial;
    size_t batch_size;
    std::string tome_path;
};