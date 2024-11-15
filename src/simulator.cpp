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

#include <gsl/gsl_rng.h>

#include <storyteller/simulator.hpp>
#include <storyteller/community.hpp>
#include <storyteller/parameters.hpp>
#include <storyteller/person.hpp>
#include <storyteller/ledger.hpp>
#include <storyteller/utility.hpp>
#include <storyteller/database_handler.hpp>

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

    // write desired metrics to the experiment database
    if (sim_flags["simulate"]) db_handler->write_metrics(ledger, par);
}

std::vector<Person*> Simulator::get_population() const {
    return community->get_population();
}