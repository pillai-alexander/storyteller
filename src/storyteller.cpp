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

#include <argh.h>
#include <nlohmann/json.hpp>

#include <storyteller/storyteller.hpp>
#include <storyteller/simulator.hpp>
#include <storyteller/parameters.hpp>
#include <storyteller/utility.hpp>
#include <storyteller/database_handler.hpp>

using json = nlohmann::json;

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
    // parse command-line arguments using Argh! library
    cmdl_args.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

    /// @todo implement #sensible_inputs() to handle all checks here before moving on

    // store all necessary program-flags
    simulation_flags["process"]  = cmdl_args["process"];
    simulation_flags["simulate"] = cmdl_args["simulate"];
    simulation_flags["particle"] = cmdl_args["particle"];
    simulation_flags["example"]  = cmdl_args["example"];
    simulation_flags["simvis"]   = cmdl_args["simvis"];

    // extract sim serial
    if (simulation_flags["particle"] and not (cmdl_args({"-s", "--serial"}) >> simulation_serial)) {
        std::cerr << "ERROR: pass serial id file after --particle using -s or --serial.";
        exit(-1);
    }

    // extract batch size if present or default to one (ie, a single simulation batch)
    cmdl_args({"-b", "--batch"}, 1) >> batch_size;

    if ((simulation_flags["process"] or simulation_flags["particle"]) and not (cmdl_args({"-f", "--file"}) >> config_file)) {
        std::cerr << "ERROR: pass config file after using -f or --file.";
        exit(-1);
    }

    if (simulation_flags["particle"] and simulation_flags["example"]) {
        std::cerr << "ERROR: particle and example cannot be used together.";
        exit(-1);
    }

    // determine what operation the user called for
    if (simulation_flags["process"] and not config_file.empty()) {
        operation_to_perform = PROCESS_CONFIG;
    } else if (simulation_flags["simulate"]) {
        if (simulation_flags["example"] and not simulation_flags["particle"]) {
            operation_to_perform = EXAMPLE_SIM;
        } else if (simulation_flags["particle"] and not config_file.empty() and simulation_serial >= 0 and not simulation_flags["example"]) {
            operation_to_perform = BATCH_SIM;
        } else {
            std::cerr << "ERROR: incorrect arguments.";
            exit(-1);
        }
    }
}

Storyteller::~Storyteller() {}

const Parameters* Storyteller::get_parameters() const { return parameters.get(); }
int Storyteller::get_serial()                   const { return simulation_serial; }
size_t Storyteller::get_batch_size()            const { return batch_size; }
std::string Storyteller::get_config_file()      const { return config_file; }
bool Storyteller::get_flag(std::string key)     const { return simulation_flags.at(key); }

void Storyteller::set_flag(std::string key, bool val) { simulation_flags[key] = val; }

int Storyteller::run() {
    switch (operation_to_perform) {
        case PROCESS_CONFIG: return construct_database();
        case EXAMPLE_SIM:    return default_simulation();
        case BATCH_SIM:      return batch_simulation();
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
int Storyteller::default_simulation() {
    init();
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
    init();
    for (size_t i = 1; i <= batch_size; ++i) {
        simulator = std::make_unique<Simulator>(parameters.get(), db_handler.get(), rng_handler.get());
        simulator->set_flags(simulation_flags);
        simulator->init();
        simulator->simulate();
        simulator->results();

        if (batch_size > 1) {
            ++simulation_serial;
            reset();
            init();
        }
    }

    if (simulation_flags["simvis"]) draw_simvis();
    return 0;
}

/**
 * @details Initializes the Storyteller appropriately for the proper type of
 *          simulation (ie, example or particle). For the example simulation,
 *          the #db_handler, #parameters, and #rng_handler objects are default-
 *          initialized. Otherwise, parameter values are read from the appropriate
 *          particle in the experiment database and used to construct the
 *          #db_handler, #parameters, and #rng_handler objects.
 */
void Storyteller::init() {
    if (simulation_flags["simulate"] and not config_file.empty()) {
        // slurp the JSON configuration file
        std::ifstream cfg_file(config_file);
        auto cfg = json::parse(cfg_file);
        cfg_file.close();

        // extract the experiment database path
        // if the user specifies a path, use that path
        // otherwise, the default value is a databse in the main.cpp directory
        // that is named using the experiment name
        std::string db_path = (cfg.contains("database_path"))
                                  ? std::string(cfg["database_path"])
                                  : std::string(cfg["experiment_name"]) + ".sqlite";

        // init a dictionary to store parameter names and values from the
        // experiment database
        std::map<std::string, double> model_params;
        for (auto& [key, el] : cfg["model_parameters"].items()) {
            model_params[el["fullname"]] = 0.0;
        }

        // store the names of the metrics to report
        std::vector<std::string> model_mets;
        for (auto& [key, el] : cfg["metrics"].items()) {
            model_mets.push_back(el["fullname"]);
        }

        // create the DatabaseHandler and read the proper parameters
        db_handler = std::make_unique<DatabaseHandler>(this, db_path);
        db_handler->read_parameters(simulation_serial, model_params);

        // create the RngHandler with the proper seed
        rng_handler = std::make_unique<RngHandler>(model_params["seed"]);

        // create the Parameters and update with the proper values
        parameters = std::make_unique<Parameters>(rng_handler.get(), model_params);
        parameters->simulation_duration = cfg["sim_duration"];
        parameters->database_path       = db_path;
        parameters->return_metrics      = model_mets;
        parameters->simulation_serial   = simulation_serial;
    } else {
        // default-initialize the DatabaseHandler, RngHandler, and Parameters
        db_handler = nullptr;
        rng_handler = std::make_unique<RngHandler>(0);
        parameters = std::make_unique<Parameters>(rng_handler.get());

    }
}

int Storyteller::construct_database() {
    if (not config_file.empty()) {
        return process_config();
    } else {
        std::cerr << "ERROR: pass config file after --process.";
        return -1;
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

/**
 * @details Constructs the experiment database using the user-provided configuration
 *          file if the database does not already exist.
 */
int Storyteller::process_config() {
    if (simulation_flags["process"] and not config_file.empty()) {
        // slurp the JSON configuration file
        std::ifstream cfg_file(config_file);
        auto cfg = json::parse(cfg_file);
        cfg_file.close();

        // extract the experiment database path
        // if the user specifies a path, use that path
        // otherwise, the default value is a databse in the main.cpp directory
        // that is named using the experiment name
        std::string db_path = (cfg.contains("database_path"))
                                  ? std::string(cfg["database_path"])
                                  : std::string(cfg["experiment_name"]) + ".sqlite";

        // create the DatabaseHandler
        db_handler = std::make_unique<DatabaseHandler>(this, db_path);
        if (db_handler->database_exists()) {
            std::cerr << "ERROR: database " << db_path << " already exists\n";
            return -1;
        } else {
            // construct the experiment database since it does not exist
            std::cerr << db_path << " does not exist. Initializing...\n";
            return db_handler->init_database(cfg);
        }
    } else {
        std::cerr << "ERROR: improper args for config processing\n"
                  << "\t cmd: exec --process -f config.json";
        return -1;
    }
}