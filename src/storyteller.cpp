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
#include <filesystem>
#include <algorithm>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <argh.h>

#include <storyteller/storyteller.hpp>
#include <storyteller/tome.hpp>
#include <storyteller/simulator.hpp>
#include <storyteller/parameters.hpp>
#include <storyteller/utility.hpp>
#include <storyteller/database_handler.hpp>
#include <storyteller/person.hpp>

namespace fs = std::filesystem;

/**
 * @details Parses command-line arguments and extracts all necessary program flags into
 *          #simulation_flags.
 */
Storyteller::Storyteller(int argc, char* argv[])
    : simulation_serial(-1),
      batch_size(1),
      tome_path(""),
      simulator(nullptr),
      operation_to_perform(NUM_OPERATION_TYPES),
      simulation_flags() {
    // inititalize the lua virtual machine using sol2 library
    lua_vm = std::make_unique<sol::state>();

    // parse command-line arguments using Argh! library
    cmdl_args.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

    // store all necessary program-flags
    simulation_flags["init"]         = cmdl_args["init"];
    simulation_flags["simulate"]     = cmdl_args["simulate"];
    simulation_flags["simvis"]       = cmdl_args["simvis"];
    simulation_flags["quiet"]        = cmdl_args[{"-q", "--quiet"}];
    simulation_flags["verbose"]      = cmdl_args[{"-v", "--verbose"}];
    simulation_flags["very_verbose"] = cmdl_args[{"-vv", "--very-verbose"}];
    simulation_flags["synthpop"]     = cmdl_args["gen-synth-pop"];
    simulation_flags["hpc_mode"]     = cmdl_args["hpc"];
    simulation_flags["hpc_slurp"]    = cmdl_args["slurp"];
    simulation_flags["hpc_clean"]    = cmdl_args["clean"];
    simulation_flags["exp_report"]   = cmdl_args["report"];

    if (simulation_flags.at("very_verbose")) simulation_flags.at("verbose") = true;

    // extract sim serial or keep default of -1 (ie, no specified serial)
    cmdl_args({"-s", "--serial"}, -1) >> simulation_serial;

    // extract batch size if present or default to one (ie, a single simulation batch)
    cmdl_args({"-b", "--batch"}, 1) >> batch_size;

    // extract core config file path or default to empty string
    cmdl_args({"-t", "--tome"}, "") >> tome_path;

    // determine what operation the user called for
    if (sensible_inputs()) {
        tome = (tome_path.empty()) ? nullptr : std::make_unique<Tome>(lua_vm.get(), tome_path);

        if (simulation_flags["init"]) {
            operation_to_perform = INITIALIZE;
        } else if (simulation_flags["simulate"]) {
            operation_to_perform = BATCH_SIM;
        } else if (simulation_flags["synthpop"]) {
            operation_to_perform = GENERATE_SYNTHETIC_POPULATION;
        } else if (simulation_flags["exp_report"]) {
            operation_to_perform = GENERATE_EXPERIMENT_REPORT;
        } else if (simulation_flags["hpc_slurp"]) {
            operation_to_perform = SLURP_CSVS_INTO_DATABASE;
        } else if (simulation_flags["hpc_clean"]) {
            operation_to_perform = CLEANUP_HPC_CSVS;
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
std::string Storyteller::get_config_file()      const { return tome_path; }
bool Storyteller::get_flag(std::string key)     const { return simulation_flags.at(key); }

void Storyteller::set_flag(std::string key, bool val) { simulation_flags[key] = val; }

bool Storyteller::sensible_inputs() const {
    int ret = 0;
    bool tome_is_set = not tome_path.empty();
    bool init        = simulation_flags.at("init");
    bool sim         = simulation_flags.at("simulate");
    bool serial      = (simulation_serial != -1) and (simulation_serial >= 0);
    bool synthpop    = simulation_flags.at("synthpop");
    bool hpc         = simulation_flags.at("hpc_mode");
    bool slurp       = simulation_flags.at("hpc_slurp");
    bool clean       = simulation_flags.at("hpc_clean");
    bool report      = simulation_flags.at("exp_report");

    // exec --tome tomefile --init
    // ret += init and tome_is_set and not sim and not example;
    ret += init and tome_is_set and not sim;

    // exec --tome tomefile --simulate --serial 0
    // exec --tome tomefile --simulate --serial 0 --batch 2
    // ret += sim and tome_is_set and not init and not example;
    ret += sim and tome_is_set and serial and not init;

    // exec --tome tomefile --gen-synth-pop --serial 0
    ret += synthpop and tome_is_set and serial and not sim;

    // exec --tome tomefile --hpc --slurp
    ret += hpc and slurp and tome_is_set and not sim and not clean;

    // exec --tome tomefile --hpc --clean
    ret += hpc and clean and tome_is_set and not sim and not slurp;

    // exec --tome tomefile --report
    ret += report and tome_is_set and not init and not sim;

    return (ret == 1);
}

int Storyteller::run() {
    switch (operation_to_perform) {
        case INITIALIZE: {
            return construct_database();
        }
        case BATCH_SIM: {
            return batch_simulation();
        }
        case GENERATE_SYNTHETIC_POPULATION: {
            init_simulation();
            auto ret = generate_synthpop();
            reset();
            return ret;
        }
        case GENERATE_EXPERIMENT_REPORT: {
            return generate_exp_report();
        }
        case SLURP_CSVS_INTO_DATABASE: {
            return slurp_metrics_files();
        }
        case CLEANUP_HPC_CSVS: {
            return cleanup_metrics_files();
        }
        default: {
            std::cerr << "No operation performed.";
            return 0;
        }
    }
}

int Storyteller::generate_synthpop() {
    std::ofstream popfile(tome->get_path("synthpop"));
    popfile << "pid,flu_suscep,nonflu_suscep,vax_status,flu_vax_protec,nonflu_vax_protec\n";

    auto pop = simulator->get_population();
    for (const auto& p : pop) {
        popfile << p->get_id() << ','
                  << p->get_susceptibility(INFLUENZA) << ','
                  << p->get_susceptibility(NON_INFLUENZA) << ','
                  << p->is_vaccinated() << ','
                  << p->get_vaccine_protection(INFLUENZA) << ','
                  << p->get_vaccine_protection(NON_INFLUENZA) << '\n';
    }
    popfile.close();

    return 0;
}


int Storyteller::generate_exp_report() {
    // create report file name (expname_version.md)
    //   - need to replace whitespace with underscores
    //   - need to replace periods with dashes
    std::string exp_name = tome->get_element_as<std::string>("experiment_name");
    std::replace(exp_name.begin(), exp_name.end(), ' ', '_');


    std::string exp_ver = tome->get_element_as<std::string>("experiment_version");
    std::replace(exp_ver.begin(), exp_ver.end(), '.', '-');

    std::string report_filename = exp_name + "_v" + exp_ver + ".md";
    fs::path report_path = tome->get_path("tome_rt") / fs::path(report_filename);

    std::ofstream report(report_path);

    // write tome.lua information
    //   - title: experiment name
    //   - description: experiment description
    //   - database file: db path
    //   - global params: n reals, par val tol
    report << "# " << tome->get_element_as<std::string>("experiment_name") << '\n'
           << "### Version: " << tome->get_element_as<std::string>("experiment_version") << '\n'
           << "## Description:\n" << tome->get_element_as<std::string>("experiment_description") << '\n'
           << '\n'
           << "## Global parameters:\n"
           << '\n'
           << "- Number of realizations per particle: " << tome->get_element_as<double>("n_realizations") << '\n'
           << "- Parameter value tolerance: " << tome->get_element_as<double>("par_value_tolerance") << '\n'
           << '\n';

    // write parameters.lua information
    //   - step params
    //   - copy params
    //   - const params
    std::map<std::string, std::vector<std::string>> par_names;
    const auto& par_table = tome->get_config_params().at("parameters").as<sol::table>();
    for (const auto& [key, p] : par_table) {
        const auto fullname = key.as<std::string>();
        const auto flag     = p.as<sol::table>().get<std::string>("flag");
        par_names[flag].push_back(fullname);
    }

    report << "## Step parameters:\n"
           << '\n'
           << "Name | Values\n"
           << "--- | ---\n";
    for (const auto& name : par_names.at("step")) {
        const auto p = par_table.get<sol::table>(name);
        auto defined_vals = p.get<sol::optional<std::vector<double>>>("values");
        std::stringstream val_text;
        if (defined_vals) {
            for (const double& v : defined_vals.value()) {
                  val_text << v << ",";
            }
        } else {
            const auto start = p.get<double>("lower");
            const auto end   = p.get<double>("upper");
            const auto step  = p.get<double>("step");

            val_text << start << " to " << end << ", (step: " << step << ")";
        }
        report << name << " | " << val_text.str() << '\n';
    }
    report << '\n';

    report << "## Const parameters:\n"
           << '\n'
           << "Name | Value\n"
           << "--- | ---\n";
    for (const auto& name : par_names.at("const")) {
        const auto p = par_table.get<sol::table>(name);
        const auto val = p.get<double>("value");
        report << name << " | " << val << '\n';
    }
    report << '\n';

    report << "## Copy parameters:\n"
           << '\n'
           << "Name | Copies\n"
           << "--- | ---\n";
    for (const auto& name : par_names.at("copy")) {
        const auto p = par_table.get<sol::table>(name);
        const auto who = p.get<std::string>("who");
        report << name << " | " << who << '\n';
    }
    report << '\n';
    report.close();

    // write metrics.lua information
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
        init_simulation();
        simulator->simulate();
        simulator->results();

        if (simulation_flags["simvis"]) {
            generate_synthpop();
            draw_simvis();
        }

        db_handler->end_job(simulation_serial);
        reset();
        ++simulation_serial;
    }
    return 0;
}

/**
 * @details Initializes the Storyteller appropriately for a simulation operation
 *          Parameter values are read from the appropriate particle in the
 *          experiment database and used to construct the #db_handler, #parameters
 *          and #rng_handler objects.
 */
void Storyteller::init_simulation() {
        db_handler = std::make_unique<DatabaseHandler>(this);
        db_handler->start_job(simulation_serial);
        rng_handler = std::make_unique<RngHandler>();
        parameters = std::make_unique<Parameters>(rng_handler.get(), db_handler.get(), tome.get());
        parameters->read_parameters_for_serial(simulation_serial);

        if (parameters->are_valid()) {
            simulator = std::make_unique<Simulator>(parameters.get(), db_handler.get(), rng_handler.get());
            simulator->set_flags(simulation_flags);
            simulator->init();
        } else {
            std::cerr << "ERROR: invalid parameters\n";
            exit(-1);
        }
}

int Storyteller::construct_database() {
    // create the DatabaseHandler
    std::string db_path = tome->get_path("database");
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
    cmd << "Rscript " << tome->get_path("simvis.R") << ' ' << tome->get_path("tome_rt");
    if (simulation_flags["verbose"]) std::cerr << "Calling `" << cmd.str() << "`\n";
    return system(cmd.str().c_str());
}

int Storyteller::slurp_metrics_files() {
    db_handler = std::make_unique<DatabaseHandler>(this);
    db_handler->drop_table_if_exists("met");

    std::stringstream cmd;
    cmd << "Rscript " << tome->get_path("slurp.R") << ' '
                      << tome->get_path("database") << ' '
                      << tome->get_path("out_dir");
    if (simulation_flags["verbose"]) std::cerr << "Calling `" << cmd.str() << "`\n";
    return system(cmd.str().c_str());

    return 0;
}

int Storyteller::cleanup_metrics_files() {
    for (auto& f : fs::directory_iterator(tome->get_path("out_dir"))) {
        std::ostringstream cmd("rm ", std::ios_base::ate);
        cmd << f.path().string();

        std::cerr << cmd.str() << '\n';
        system(cmd.str().c_str());

        cmd.str(std::string());
    }

    return 0;
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