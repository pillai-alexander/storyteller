/**
 * @file simulator.hpp
 * @author Alexander N. Pillai
 * @brief Contains the Simulator class that is responsible for performing a single
 *        simulation (either the default example or a parameterized particle).
 *
 * @copyright TBD
 */
#pragma once

#include <memory>
#include <string>
#include <map>

#include <gsl/gsl_rng.h>

class Parameters;
class Community;
class Infection;
class Ledger;
class DatabaseHandler;
class RngHandler;

/**
 * @brief Main simulation object that handles simulation setup, performs the
 *        actual simulation itself, and processes simualtion data for the
 *        experiment database.
 *
 * A Simulator object is created by the Storyteller for each simulation to be
 * performed. The object then prepares for, runs, and reports the data from the
 * specified simulation.
 */
class Simulator {
  public:
    /**
     * @brief Construct a new Simulator object with the Parameters, DatabaseHandler,
     *        and RngHandler provided by the Storyteller.
     *
     * @param parameters Parameters object owned by the Storyteller
     * @param dbh Database object owned by the Storyteller
     * @param rngh RngHandler object owned by the Storyteller
     */
    Simulator(const Parameters* parameters, DatabaseHandler* dbh, const RngHandler* rngh);
    ~Simulator();

    /**
     * @brief Store the program flags parsed by the Storyteller
     *
     * @param flags Dictionary of program flags
     */
    void set_flags(std::map<std::string, bool> flags);

    /**
     * @brief Perform the necessary tasks to initialize a simulation.
     */
    void init();

    /**
     * @brief Perform the simulation itself.
     */
    void simulate();

    /**
     * @brief Perform any necessary post-simulation processing and report requested
     *        simulation metrics.
     */
    void results();

  private:
    /**
     * @brief Helper function that contains all simulation tasks that need to be
     *        executed at each simulated time step.
     */
    void tick();

    size_t sim_time;                        ///< Current simulation time step
    std::map<std::string, bool> sim_flags;  ///< Program flags provided by the Storyteller

    std::unique_ptr<Community> community;   ///< Created for each simulation
    const RngHandler* rng_handler;          ///< Points to #Storyteller::rng_handler
    const Parameters* par;                  ///< Points to #Storyteller::parameters
    DatabaseHandler* db_handler;      ///< Points to #Storyteller::db_handler
};