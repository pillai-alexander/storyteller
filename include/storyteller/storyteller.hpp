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

/**
 * @brief Defines the types of operations that the Storyteller is capable of
 *        performing.
 */
enum OperationType {
    PROCESS_CONFIG,
    EXAMPLE_SIM,
    BATCH_SIM,
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
     * @brief Initialize Storteller for running a simulation.
     */
    void init();

    /**
     * @brief Process configuration file and construct the database if it does
     *        not already exist.
     *
     * @return int Return code (0 if sucessful)
     */
    int process_config();

    /**
     * @brief Constructs a new experiment database given the user-provided
     *        configuration file.
     *
     * @return int Return code (0 if sucessful)
     */
    int construct_database();

    /**
     * @brief Built-in example simulation that does not require a configuration
     *        file.
     *
     * @return int Return code (0 if sucessful)
     */
    int default_simulation();

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

    std::unique_ptr<Simulator> simulator;           ///< Created for each simulation to be run
    std::unique_ptr<DatabaseHandler> db_handler;    ///< Handles all database operations
    std::unique_ptr<RngHandler> rng_handler;        ///< Handles all pseudo-random number generation
    std::unique_ptr<Parameters> parameters;         ///< Stores all necessary simulation parameters

    OperationType operation_to_perform;

    argh::parser cmdl_args;                         ///< Stores parsed command-line arguments
    std::map<std::string, bool> simulation_flags;   ///< Stores program flags

    int simulation_serial;
    size_t batch_size;
    std::string config_file;
};