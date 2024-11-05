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
    simulation_flags["example"]  = cmdl_args["example"];
    simulation_flags["simvis"]   = cmdl_args["simvis"];

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
        } else if (simulation_flags["example"]) {
            operation_to_perform = EXAMPLE_SIM;
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
    bool example     = simulation_flags.at("example");
    bool sim         = simulation_flags.at("simulate");

    // exec --example
    ret += example and not sim and not tome_is_set;

    // exec --tome tomefile --init
    ret += init and tome_is_set and not sim and not example;

    // exec --tome tomefile --simulate --serial 0
    // exec --tome tomefile --simulate --serial 0 --batch 2
    ret += sim and tome_is_set and not init and not example;

    return (ret == 1);
}

int Storyteller::run() {
    switch (operation_to_perform) {
        case INITIALIZE:  return construct_database();
        case EXAMPLE_SIM: return example_simulation();
        case BATCH_SIM:   return batch_simulation();
        default: {
            std::cerr << "No operation performed.";
            return 0;   
        }
    }
}

/**
 * @details Performs a built-in simulation that uses all of the default parameter
 *          values and that does not require any user-provided configuration nor
 *          an experiment database. For this operation, #init default-initializes
 *          the #db_handler, #parameters, and #rng_handler objects.
 */
int Storyteller::example_simulation() {
    db_handler = nullptr;
    rng_handler = std::make_unique<RngHandler>(0);
    parameters = std::make_unique<Parameters>(rng_handler.get());

    simulator = std::make_unique<Simulator>(parameters.get(), db_handler.get(), rng_handler.get());
    simulator->set_flags(simulation_flags);
    simulator->init();
    simulator->simulate();
    simulator->results();

    if (simulation_flags["simvis"]) draw_simvis();
    return 0;
}

/**
 * @details Performs a batch of simulations that require an experiment database
 *          for parameterization. For each simulation, the Storyteller is initialized
 *          using the proper particle from the database (which also correctly
 *          initializes the #db_handler, #parameters, and #rng_handler objects for
 *          each simulation in the batch).
 */
int Storyteller::batch_simulation() {
    init_batch();
    for (size_t i = 1; i <= batch_size; ++i) {
        init_simulation(db_handler->params_for_serial(simulation_serial));
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
    for (size_t i = 0; i < batch_size; ++i) {
        db_handler->add_serial(simulation_serial + i);
    }
    db_handler->read_parameters();
}

void Storyteller::init_simulation(std::map<std::string, double> sim_params) {
    db_handler->start_job(simulation_serial);
    // create the RngHandler with the proper seed
    rng_handler = std::make_unique<RngHandler>(sim_params["seed"]);

    // create the Parameters and update with the proper values
    parameters = std::make_unique<Parameters>(rng_handler.get(), sim_params);
    parameters->simulation_duration = tome->get_element_as<size_t>("sim_duration");
    parameters->database_path       = tome->database_path();
    parameters->simulation_serial   = simulation_serial;
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
    rng_handler.reset(nullptr);
    parameters.reset(nullptr);
}