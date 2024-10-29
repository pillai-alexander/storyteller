#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <memory>

#include "argh.hpp"
#include <sol/sol.hpp>

#include <storyteller/simulator.hpp>

// process config file
    // --process -f config.json
    // --process --file config.json
// simulate default example
    // --simulate --example
// simulate particle from database
    // --simulate -f config.json --particle -s 0
    // --simulate --file config.json --particle --serial 0
// simulate multiple particles from database starting with specified serial
    // --simulate -f config.json --particle -s 0 -b 2
    // --simulate --file config.json --particle --serial 0 --batch 2

int main(int argc, char* argv[]) {
    argh::parser cmdl;
    cmdl.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

    std::map<std::string, bool> sim_flags;
    sim_flags["process"]  = cmdl["process"];
    sim_flags["simulate"] = cmdl["simulate"];
    sim_flags["particle"] = cmdl["particle"];
    sim_flags["example"]  = cmdl["example"];
    sim_flags["simvis"]   = cmdl["simvis"];

    int serial = -1;
    if (sim_flags["particle"] and not (cmdl({"-s", "--serial"}) >> serial)) {
        std::cerr << "ERROR: pass serial id file after --particle using -s or --serial.";
        exit(-1);
    }

    unsigned int batch = 1;
    cmdl({"-b", "--batch"}, 1) >> batch;
    std::cerr << "num in batch: " << batch << '\n';

    std::string config_file("");
    if ((sim_flags["process"] or sim_flags["particle"]) and not (cmdl({"-f", "--file"}) >> config_file)) {
        std::cerr << "ERROR: pass config file after using -f or --file.";
        exit(-1);
    }

    std::unique_ptr<Simulator> sim = nullptr;
    bool ready_to_simulate = false;
    if (sim_flags["process"]) {
        if (not config_file.empty()) {
            std::stringstream cmd;
            cmd << "Rscript process_config.R " << config_file;
            std::cerr << "Calling `" << cmd.str() << "`\n";
            int ret = system(cmd.str().c_str());
            if (ret == -1) {
                std::cerr << "System call to `" << cmd.str() << "` failed\n";
            }
        } else {
            std::cerr << "ERROR: pass config file after --process.";
            exit(-1);
        }

    } else if (sim_flags["simulate"]) {
        if (sim_flags["particle"] and sim_flags["example"]) {
            std::cerr << "ERROR: particle and example cannot be used together.";
            exit(-1);
        } else if (sim_flags["example"]) {
            sim = std::make_unique<Simulator>();
            ready_to_simulate = true;
        } else if (sim_flags["particle"] and (serial >= 0) and not config_file.empty()) {
            // sim = std::make_unique<Simulator>(config_file, serial);
            ready_to_simulate = true;
        } else {
            std::cerr << "ERROR: incorrect arguments.";
            exit(-1);
        }

        if (ready_to_simulate) {
            for (unsigned int rep = 1; rep <= batch; ++rep) {
                std::cerr << "runing sim " << rep << " of " << batch << " serial: " << serial << '\n';
                if (not sim and (sim_flags["particle"] and (serial >= 0) and not config_file.empty())) {
                    sim = std::make_unique<Simulator>(config_file, serial);
                }
                sim->set_flags(sim_flags);
                // construct community
                sim->init();
                // simulate
                sim->simulate();
                // return desired metrics
                sim->results();
                ++serial;
                sim = nullptr;
            }
        }

        if (sim_flags["simvis"]) {
            std::stringstream cmd;
            cmd << "Rscript figs/simvis.R";
            std::cerr << "Calling `" << cmd.str() << "`\n";
            int ret = system(cmd.str().c_str());
            if (ret == -1) {
                std::cerr << "System call to `" << cmd.str() << "` failed\n";
            }
        }
    } else {
        std::cerr << "No flags set.";
        exit(0);
    }
    return 0;
}