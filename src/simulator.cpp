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

Simulator::Simulator(const Parameters* parameters, const DatabaseHandler* dbh, const RngHandler* rngh) : sim_time(0) {
    rng_handler = rngh;
    par         = parameters;
    db_handler  = dbh;
    community   = std::make_unique<Community>(par, rngh);
}

Simulator::~Simulator() {}

void Simulator::set_flags(std::map<std::string, bool> flags) { sim_flags = flags; }

void Simulator::init() {
    community->vaccinate_population(sim_time);
}

void Simulator::simulate() {
    for (; sim_time < par->simulation_duration; ++sim_time) {
        tick();
    }
}

void Simulator::tick() {
    community->transmission(sim_time);
}

void Simulator::results() {
    auto ledger = community->ledger.get();

    auto total_vaxd_flu_infs = ledger->total_infections(VACCINATED, INFLUENZA);
    auto total_vaxd_flu_cases = ledger->total_sympt_infections(VACCINATED, INFLUENZA);
    auto total_vaxd_flu_mai = ledger->total_mai(VACCINATED, INFLUENZA);
    auto total_vaxd_nonflu_infs = ledger->total_infections(VACCINATED, NON_INFLUENZA);
    auto total_vaxd_nonflu_cases = ledger->total_sympt_infections(VACCINATED, NON_INFLUENZA);
    auto total_vaxd_nonflu_mai = ledger->total_mai(VACCINATED, NON_INFLUENZA);
    auto vax_coverage = (double) ledger->total_vaccinations() / par->population_size;
    auto final_tnd_ve = ledger->get_tnd_ve_est(par->simulation_duration - 1);

    std::cerr << "rng seed:            " << rng_handler->get_seed() << '\n'
              << "flu infs (cAR%):     " << total_vaxd_flu_infs << " (" << ((double) total_vaxd_flu_infs/par->population_size)*100 << "%)" << '\n'
              << "flu cases (inf%):    " << total_vaxd_flu_cases << " (" << ((double) total_vaxd_flu_cases/total_vaxd_flu_infs)*100 << "%)" << '\n'
              << "flu mais (inf%):     " << total_vaxd_flu_mai << " (" << ((double) total_vaxd_flu_mai/total_vaxd_flu_infs)*100 << "%)" << '\n'
              << "nonflu infs (cAR%):  " << total_vaxd_nonflu_infs << " (" << ((double) total_vaxd_nonflu_infs/par->population_size)*100 << "%)" << '\n'
              << "nonflu cases (inf%): " << total_vaxd_nonflu_cases << " (" << ((double) total_vaxd_nonflu_cases/total_vaxd_nonflu_infs)*100 << "%)" << '\n'
              << "nonflu mais (inf%):  " << total_vaxd_nonflu_mai << " (" << ((double) total_vaxd_nonflu_mai/total_vaxd_nonflu_infs)*100 << "%)" << '\n'
              << "final tnd ve (vax%): " << final_tnd_ve << " ("<< vax_coverage*100 << "%)" << '\n';

    ledger->calculate_cumulatives();
    ledger->calculate_tnd_ve_est();

    // { // generate linelist csv (sim.linelist)
    //     ledger->generate_linelist_csv();
    // }

    if (sim_flags["simvis"]) ledger->generate_simvis_csv();

    if (sim_flags["particle"]) db_handler->write_metrics(ledger, par);
}