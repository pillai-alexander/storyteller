/**
 * @file simulator.cpp
 * @author Alexander N. Pillai
 * @brief Contains the Simulator class that is responsible for performing a single
 *        simulation (either the default example or a parameterized particle).
 *
 * @copyright TBD
 */
#include <memory>
#include <iostream>
#include <fstream>
#include <utility>
#include <string>
#include <map>
#include <sstream>
#include <filesystem>

#include <gsl/gsl_rng.h>

#include <storyteller/simulator.hpp>
#include <storyteller/community.hpp>
#include <storyteller/parameters.hpp>
#include <storyteller/person.hpp>
#include <storyteller/ledger.hpp>
#include <storyteller/utility.hpp>
#include <storyteller/database_handler.hpp>
#include <storyteller/tome.hpp>

namespace fs = std::filesystem;

Simulator::Simulator(const Parameters* parameters, DatabaseHandler* dbh, const RngHandler* rngh)
    : sim_time(0),
      rng_handler(rngh),
      par(parameters),
      db_handler(dbh) {
    community = std::make_unique<Community>(par, rngh);
}

Simulator::~Simulator() {}

void Simulator::set_flags(std::map<std::string, bool> flags) { sim_flags = flags; }

void Simulator::init() {
    // vaccinate population before transmission starts
    community->vaccinate_population(sim_time);
}

/**
 * @details Main function of the simulation that houses the core simulation loop.
 */
void Simulator::simulate() {
    // core simulation loop
    for (; sim_time < par->get("sim_duration"); ++sim_time) {
        tick();
    }
}

void Simulator::tick() {
    community->transmission(sim_time);
}

void Simulator::results() {
    // retrive the ledger from the community
    /// @todo the ledger should be owned by the simulator but the community can access it
    auto ledger = community->ledger.get();

    // perform necessary simulation data processing
    ledger->calculate_cumulatives();
    ledger->calculate_tnd_ve_est();

    // retrieve desired metrics
    if (sim_flags["verbose"]) {
        if (sim_flags.at("very_verbose")) {
            std::cerr << "t\tc_vaxflu_mais\tc_unvaxflu_mais\tc_vaxnflu_mais\tc_unvaxnflu_mais\ttnd_ve\n";
            for (size_t t = 0; t < par->get("sim_duration"); ++t) {
                std::cerr << t << '\t'
                          << ledger->get_cumul_mais(VACCINATED, INFLUENZA, t) << "\t\t"
                          << ledger->get_cumul_mais(UNVACCINATED, INFLUENZA, t) << "\t\t"
                          << ledger->get_cumul_mais(VACCINATED, NON_INFLUENZA, t) << "\t\t"
                          << ledger->get_cumul_mais(UNVACCINATED, NON_INFLUENZA, t) << "\t\t\t"
                          << ledger->get_tnd_ve_est(t) << '\n';
            }
        }
        auto pop_size = par->get("pop_size");

        auto total_vaxd_flu_infs = ledger->total_infections(VACCINATED, INFLUENZA);
        auto total_vaxd_flu_cases = ledger->total_sympt_infections(VACCINATED, INFLUENZA);
        auto total_vaxd_flu_mai = ledger->total_mai(VACCINATED, INFLUENZA);
        auto total_vaxd_nonflu_infs = ledger->total_infections(VACCINATED, NON_INFLUENZA);
        auto total_vaxd_nonflu_cases = ledger->total_sympt_infections(VACCINATED, NON_INFLUENZA);
        auto total_vaxd_nonflu_mai = ledger->total_mai(VACCINATED, NON_INFLUENZA);
        auto vax_coverage = (double) ledger->total_vaccinations() / pop_size;
        auto final_tnd_ve = ledger->get_tnd_ve_est(par->get("sim_duration") - 1);

        // print basic simulation results to the terminal
        std::cerr << "rng seed:            " << rng_handler->get_seed() << '\n'
                << "vaxd flu infs (cAR%):     " << total_vaxd_flu_infs << " (" << ((double) total_vaxd_flu_infs/pop_size)*100 << "%)" << '\n'
                << "vaxd flu cases (inf%):    " << total_vaxd_flu_cases << " (" << ((double) total_vaxd_flu_cases/total_vaxd_flu_infs)*100 << "%)" << '\n'
                << "vaxd flu mais (inf%):     " << total_vaxd_flu_mai << " (" << ((double) total_vaxd_flu_mai/total_vaxd_flu_infs)*100 << "%)" << '\n'
                << "vaxd nonflu infs (cAR%):  " << total_vaxd_nonflu_infs << " (" << ((double) total_vaxd_nonflu_infs/pop_size)*100 << "%)" << '\n'
                << "vaxd nonflu cases (inf%): " << total_vaxd_nonflu_cases << " (" << ((double) total_vaxd_nonflu_cases/total_vaxd_nonflu_infs)*100 << "%)" << '\n'
                << "vaxd nonflu mais (inf%):  " << total_vaxd_nonflu_mai << " (" << ((double) total_vaxd_nonflu_mai/total_vaxd_nonflu_infs)*100 << "%)" << '\n'
                << "final tnd ve (vax%):      " << final_tnd_ve << " ("<< vax_coverage*100 << "%)" << '\n';
    }

    // { // generate linelist csv (sim.linelist)
    //     ledger->generate_linelist_csv();
    // }

    // generate the simulation dashboard if requested by the user
    if (sim_flags["simvis"]) ledger->generate_simvis_csv();

    // output metrics
    if (sim_flags["simulate"]) {
        if (sim_flags["hpc_mode"]) {
            // write desired metrics to a csv file
            write_metrics_csv();
        } else {
            // write desired metrics to the experiment database
            db_handler->write_metrics(ledger, par);
        }
    }
}

std::vector<Person*> Simulator::get_population() const {
    return community->get_population();
}

void Simulator::write_metrics_csv() {
    auto file_name = "metrics_" + std::to_string(par->simulation_serial) + ".csv";
    auto file_path = fs::path(par->tome->get_path("out_dir")) / file_name;

    size_t n_rows = par->get("sim_duration");
    std::vector<std::string> csv_rows_to_write(n_rows + 1);
    csv_rows_to_write[0] = "serial,time,c_vax_flu_inf,c_vax_nonflu_inf,c_unvax_flu_inf,c_unvax_nonflu_inf,c_vax_flu_mai,c_vax_nonflu_mai,c_unvax_flu_mai,c_unvax_nonflu_mai,tnd_ve_est\n";

    auto ledger = community->ledger.get();
    std::stringstream row;
    for (size_t t = 0; t < n_rows; ++t) {
        row << par->simulation_serial << ","
            << t << ","
            << ledger->get_cumul_infs(VACCINATED, INFLUENZA, t) << ","
            << ledger->get_cumul_infs(VACCINATED, NON_INFLUENZA, t) << ","
            << ledger->get_cumul_infs(UNVACCINATED, INFLUENZA, t) << ","
            << ledger->get_cumul_infs(UNVACCINATED, NON_INFLUENZA, t) << ","
            << ledger->get_cumul_mais(VACCINATED, INFLUENZA, t) << ","
            << ledger->get_cumul_mais(VACCINATED, NON_INFLUENZA, t) << ","
            << ledger->get_cumul_mais(UNVACCINATED, INFLUENZA, t) << ","
            << ledger->get_cumul_mais(UNVACCINATED, NON_INFLUENZA, t) << ","
            << ledger->get_tnd_ve_est(t) << "\n";

        csv_rows_to_write[t + 1] = row.str();
        row.str(std::string());
    }

    std::ofstream file(file_path);
    for (auto& row : csv_rows_to_write) {
        file << row;
    }
    file.close();

    std::cerr << "csv written...\n";
}