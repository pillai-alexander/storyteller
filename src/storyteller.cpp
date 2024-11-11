/**
 * @file storyteller.cpp
 * @author Alexander N. Pillai
 * @brief Contains the Storyteller class that is responsible for taking in user
 *        input and performing the desired operations (eg, database initialization,
 *        simulations)
 *
 * @copyright TBD
 */
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <memory>
#include <fstream>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <argh.h>

#include <storyteller/storyteller.hpp>
#include <storyteller/tome.hpp>
#include <storyteller/simulator.hpp>
#include <storyteller/parameters.hpp>
#include <storyteller/utility.hpp>
#include <storyteller/database_handler.hpp>

/**
 * @details Parses command-line arguments and extracts all necessary program flags into
 *          #simulation_flags.
 */
Storyteller::Storyteller(int argc, char* argv[])
    : simulation_serial(-1),
      batch_size(1),
      config_file(""),
      simulator(nullptr),
      operation_to_perform(NUM_OPERATION_TYPES),
      simulation_flags() {
    // inititalize the lua virtual machine using sol2 library
    lua_vm = std::make_unique<sol::state>();

    // parse command-line arguments using Argh! library
    cmdl_args.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

    // store all necessary program-flags
    simulation_flags["init"]     = cmdl_args["init"];
    simulation_flags["simulate"] = cmdl_args["simulate"];
    simulation_flags["simvis"]   = cmdl_args["simvis"];
    simulation_flags["verbose"]  = cmdl_args[{"-v", "--verbose"}];

    // extract sim serial or keep default of -1 (ie, no specified serial)
    cmdl_args({"-s", "--serial"}, -1) >> simulation_serial;

    // extract batch size if present or default to one (ie, a single simulation batch)
    cmdl_args({"-b", "--batch"}, 1) >> batch_size;

    // extract core config file path or default to empty string
    cmdl_args({"-t", "--tome"}, "") >> config_file;

    // determine what operation the user called for
    if (sensible_inputs()) {
        tome = (config_file.empty()) ? nullptr : std::make_unique<Tome>(lua_vm.get(), config_file);

        if (simulation_flags["init"]) {
            operation_to_perform = INITIALIZE;
        } else if (simulation_flags["simulate"]) {
            operation_to_perform = BATCH_SIM;
        } else {
            operation_to_perform = NUM_OPERATION_TYPES;
        }
    } else {
        std::cerr << "ERROR: CLI arguments are not correct.\n";
        operation_to_perform = NUM_OPERATION_TYPES;
    }
}

Storyteller::~Storyteller() {
    if(tome) tome->clean();
}

const Parameters* Storyteller::get_parameters() const { return parameters.get(); }
const Tome* Storyteller::get_tome()             const { return tome.get(); }
int Storyteller::get_serial()                   const { return simulation_serial; }
size_t Storyteller::get_batch_size()            const { return batch_size; }
std::string Storyteller::get_config_file()      const { return config_file; }
bool Storyteller::get_flag(std::string key)     const { return simulation_flags.at(key); }

void Storyteller::set_flag(std::string key, bool val) { simulation_flags[key] = val; }

bool Storyteller::sensible_inputs() const {
    int ret = 0;
    bool tome_is_set = not config_file.empty();
    bool init        = simulation_flags.at("init");
    bool sim         = simulation_flags.at("simulate");
    bool serial      = (simulation_serial != -1) and (simulation_serial >= 0);

    // exec --tome tomefile --init
    // ret += init and tome_is_set and not sim and not example;
    ret += init and tome_is_set and not sim;

    // exec --tome tomefile --simulate --serial 0
    // exec --tome tomefile --simulate --serial 0 --batch 2
    // ret += sim and tome_is_set and not init and not example;
    ret += sim and tome_is_set and serial and not init;

    return (ret == 1);
}

int Storyteller::run() {
    switch (operation_to_perform) {
        case INITIALIZE:  return construct_database();
        case BATCH_SIM:   return batch_simulation();
        default: {
            std::cerr << "No operation performed.";
            return 0;   
        }
    }
}

/**
 * @details Performs a batch of simulations that require an experiment database
 *          for parameterization. For each simulation, the Storyteller is initialized
 *          using the proper particle from the database (which also correctly
 *          initializes the #db_handler, #parameters, and #rng_handler objects for
 *          each simulation in the batch).
 */
int Storyteller::batch_simulation() {
    for (size_t i = 1; i <= batch_size; ++i) {
        init_batch();
        simulator = std::make_unique<Simulator>(parameters.get(), db_handler.get(), rng_handler.get());
        simulator->set_flags(simulation_flags);
        simulator->init();
        simulator->simulate();
        simulator->results();
        reset();
        ++simulation_serial;
    }

    if (simulation_flags["simvis"]) draw_simvis();
    return 0;
}

/**
 * @details Initializes the Storyteller appropriately for a batch simulation
 *          operation. Parameter values are read from the appropriate particle
 *          in the experiment database and used to construct the #db_handler,
 *          #parameters, and #rng_handler objects.
 */
void Storyteller::init_batch() {
        db_handler = std::make_unique<DatabaseHandler>(this);
        rng_handler = std::make_unique<RngHandler>();
        parameters = std::make_unique<Parameters>(rng_handler.get(), db_handler.get(), tome.get());
        parameters->read_parameters_for_serial(simulation_serial);
}

int Storyteller::construct_database() {
    // create the DatabaseHandler
    std::string db_path = tome->database_path();
    db_handler = std::make_unique<DatabaseHandler>(this);

    if (db_handler->database_exists()) {
        std::cerr << "ERROR: database " << db_path << " already exists\n";
        return -1;
    } else {
        // construct the experiment database since it does not exist
        std::cerr << db_path << " does not exist. Initializing...\n";
        return db_handler->init_database();
    }
}

int Storyteller::draw_simvis() {
    std::stringstream cmd;
    cmd << "Rscript figs/simvis.R";
    std::cerr << "Calling `" << cmd.str() << "`\n";
    return system(cmd.str().c_str());
}

/**
 * @details Deletes the current #simulator, #db_handler, #parameters, and
 *          #rng_handler objects after a simulation is finished so that they can
 *          be properly initialized for the next simulation in the batch.
 */
void Storyteller::reset() {
    simulator.reset(nullptr);
    db_handler.reset(nullptr);
    rng_handler.reset(nullptr);
    parameters.reset(nullptr);
}