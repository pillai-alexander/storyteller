#include <iostream>
#include <sstream>
#include <map>
#include <memory>

#include "argh.hpp"

#include "simulator.hpp"

// process config file
    // --process -f config.json
    // --process --file config.json
// simulate default example
    // --simulate --example
// simulate particle from database
    // --simulate -f config.json --particle -s 0
    // --simulate --file config.json --particle --serial 0

int main(int argc, char* argv[]) {
    argh::parser cmdl;
    cmdl.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

    bool process  = cmdl["process"];
    bool simulate = cmdl["simulate"];
    bool particle = cmdl["particle"];
    bool example  = cmdl["example"];
    bool simvis   = cmdl["simvis"];

    int serial = -1;
    if (particle and not (cmdl({"-s", "--serial"}) >> serial)) {
        std::cerr << "ERROR: pass serial id file after --particle using -s or --serial.";
            exit(-1);
    }

    std::string config_file("");
    if ((process or particle) and not (cmdl({"-f", "--file"}) >> config_file)) {
        std::cerr << "ERROR: pass config file after using -f or --file.";
            exit(-1);
    }

    std::unique_ptr<Simulator> sim = nullptr;
    bool ready_to_simulate = false;
    if (process) {
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

    } else if (simulate) {
        if (particle and example) {
            std::cerr << "ERROR: particle and example cannot be used together.";
            exit(-1);
        } else if (example) {
            sim = std::make_unique<Simulator>();
            ready_to_simulate = true;
        } else if (particle and (serial >= 0) and not config_file.empty()) {
            sim = std::make_unique<Simulator>(config_file, serial);
            ready_to_simulate = true;
        } else {
            std::cerr << "ERROR: incorrect arguments.";
            exit(-1);
        }

        if (ready_to_simulate) {
            // construct community
            sim->init();
            // simulate
            sim->simulate();
            // return desired metrics
            sim->results();
        }

        if (simvis) {
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