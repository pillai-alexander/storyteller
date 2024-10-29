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

using json = nlohmann::json;

Storyteller::Storyteller(int argc, char* argv[])
    : simulation_serial(-1),
      batch_size(1),
      config_file(""),
      simulator(nullptr),
      operation_to_perform(NUM_OPERATION_TYPES),
      simulation_flags() {
    cmdl_args.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

    simulation_flags["process"]  = cmdl_args["process"];
    simulation_flags["simulate"] = cmdl_args["simulate"];
    simulation_flags["particle"] = cmdl_args["particle"];
    simulation_flags["example"]  = cmdl_args["example"];
    simulation_flags["simvis"]   = cmdl_args["simvis"];

    if (simulation_flags["particle"] and not (cmdl_args({"-s", "--serial"}) >> simulation_serial)) {
        std::cerr << "ERROR: pass serial id file after --particle using -s or --serial.";
        exit(-1);
    }

    cmdl_args({"-b", "--batch"}, 1) >> batch_size;

    if ((simulation_flags["process"] or simulation_flags["particle"]) and not (cmdl_args({"-f", "--file"}) >> config_file)) {
        std::cerr << "ERROR: pass config file after using -f or --file.";
        exit(-1);
    }

    if (simulation_flags["particle"] and simulation_flags["example"]) {
        std::cerr << "ERROR: particle and example cannot be used together.";
        exit(-1);
    }

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

    process_config();
}

Storyteller::~Storyteller() {}

int Storyteller::get_serial() const { return simulation_serial; }
size_t Storyteller::get_batch_size() const { return batch_size; }
std::string Storyteller::get_config_file() const { return config_file; }

bool Storyteller::get_flag(std::string key) const { return simulation_flags.at(key); }
void Storyteller::set_flag(std::string key, bool val) { simulation_flags[key] = val; }

int Storyteller::run() {
    switch (operation_to_perform) {
        case PROCESS_CONFIG: return construct_database();
        case EXAMPLE_SIM: return default_simulation();
        case BATCH_SIM: return batch_simulation();
        default: {
            std::cerr << "No operation performed.";
            return 0;
        }
    }
}

int Storyteller::default_simulation() {
    simulator = std::make_unique<Simulator>(parameters.get(), db_handler.get(), rng_handler.get());
    simulator->set_flags(simulation_flags);
    simulator->init();
    simulator->simulate();
    simulator->results();

    if (simulation_flags["simvis"]) draw_simvis();
    return 0;
}

int Storyteller::batch_simulation() {
    for (size_t i = 1; i <= batch_size; ++i) {
        simulator = std::make_unique<Simulator>(parameters.get(), db_handler.get(), rng_handler.get());
        simulator->set_flags(simulation_flags);
        simulator->init();
        simulator->simulate();
        simulator->results();

        if (batch_size > 1) {
            ++simulation_serial;
            reset();
            process_config();
        }
    }

    if (simulation_flags["simvis"]) draw_simvis();
    return 0;
}

void Storyteller::process_config() {
    if (simulation_flags["simulate"] and not config_file.empty()) {
        std::ifstream cfg_file(config_file);
        auto cfg = json::parse(cfg_file);
        cfg_file.close();

        std::string db_path = cfg["experiment_name"];
        db_path += std::string(".sqlite");

        std::map<std::string, double> model_params;
        for (auto& [key, el] : cfg["model_parameters"].items()) {
            model_params[el["fullname"]] = 0.0;
        }

        std::vector<std::string> model_mets;
        for (auto& [key, el] : cfg["metrics"].items()) {
            model_mets.push_back(el["fullname"]);
        }
        db_handler = std::make_unique<DatabaseHandler>(this, db_path);
        db_handler->read_parameters(simulation_serial, model_params);

        rng_handler = std::make_unique<RngHandler>(model_params["seed"]);

        parameters = std::make_unique<Parameters>(rng_handler.get(), model_params);
        parameters->simulation_duration = cfg["sim_duration"];
        parameters->database_path       = db_path;
        parameters->return_metrics      = model_mets;
        parameters->simulation_serial   = simulation_serial;
    } else {
        db_handler = nullptr;
        rng_handler = std::make_unique<RngHandler>(0);
        parameters = std::make_unique<Parameters>(rng_handler.get());

    }
}

int Storyteller::construct_database() {
    if (not config_file.empty()) {
        std::stringstream cmd;
        cmd << "Rscript process_config.R " << config_file;
        std::cerr << "Calling `" << cmd.str() << "`\n";
        return system(cmd.str().c_str());
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

void Storyteller::reset() {
    simulator.reset(nullptr);
    db_handler.reset(nullptr);
    rng_handler.reset(nullptr);
    parameters.reset(nullptr);
}