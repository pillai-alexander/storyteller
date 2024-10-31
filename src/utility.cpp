#include <chrono>
#include <thread>
#include <iostream>

#include <SQLiteCpp/SQLiteCpp.h>

#include <storyteller/utility.hpp>
#include <storyteller/simulator.hpp>
#include <storyteller/ledger.hpp>
#include <storyteller/storyteller.hpp>

using std::chrono::duration;
using std::chrono::milliseconds;

namespace constants {
    unsigned int ZERO = 0;
    unsigned int ONE = 1;
}

namespace util {
    double gamma_scale_from_mean(double shape, double mean) {
        return (shape == constants::ZERO) ? mean : mean / shape;
    }
}

RngHandler::RngHandler(unsigned long int seed) : rng_seed(seed) {
    infection_rng   = gsl_rng_alloc(gsl_rng_mt19937);
    vaccination_rng = gsl_rng_alloc(gsl_rng_mt19937);
    behavior_rng    = gsl_rng_alloc(gsl_rng_mt19937);

    gsl_rng_set(infection_rng, seed);
    gsl_rng_set(vaccination_rng, seed);
    gsl_rng_set(behavior_rng, seed);
}

RngHandler::~RngHandler() {
    gsl_rng_free(infection_rng);
    gsl_rng_free(vaccination_rng);
    gsl_rng_free(behavior_rng);
}

double RngHandler::draw_from_rng(RngType type) const {
    switch (type) {
        case INFECTION:   { return gsl_rng_uniform(infection_rng); }
        case VACCINATION: { return gsl_rng_uniform(vaccination_rng); }
        case BEHAVIOR:    { return gsl_rng_uniform(behavior_rng); }
        default:          { return gsl_rng_uniform(infection_rng); }
    }
}

gsl_rng* RngHandler::get_rng(RngType type) const {
    switch (type) {
        case INFECTION:   { return infection_rng; }
        case VACCINATION: { return vaccination_rng; }
        case BEHAVIOR:    { return behavior_rng; }
        default:          { return infection_rng; }
    }
}

unsigned long int RngHandler::get_seed() const { return rng_seed; }

DatabaseHandler::DatabaseHandler(const Storyteller* storyteller, std::string db_path) 
    : n_transaction_attempts(10),
      ms_delay_between_attempts(1000) {
    // owner = storyteller;
    database_path = db_path;
}

DatabaseHandler::~DatabaseHandler() {}

void DatabaseHandler::read_parameters(unsigned int serial, std::map<std::string, double>& pars) {
    for (size_t i = 0; i < n_transaction_attempts; ++i) {
        try {
            {
                SQLite::Database db(database_path);
                SQLite::Statement query(db, "SELECT * FROM par WHERE serial = ?");
                query.bind(1, (unsigned int) serial);
                while (query.executeStep()) {
                    pars["seed"] = query.getColumn("seed");
                    for (auto& [param, val] : pars) {
                        pars[param] = query.getColumn(param.c_str());
                    }
                }
            }
            std::cerr << "Read attempt " << i << " succeeded." << '\n';
            break;
        } catch (std::exception& e) {
            std::cerr << "Read attempt " << i << " failed:" << '\n';
            std::cerr << "\tSQLite exception: " << e.what() << '\n';
            std::this_thread::sleep_for(std::chrono::milliseconds(ms_delay_between_attempts));
        }
    }
}

std::vector<std::string> DatabaseHandler::prepare_insert_sql(const Ledger* ledger, const Parameters* par) const {
    std::stringstream cols;
    cols << "serial";
    for (auto& col : par->return_metrics) {
        cols << "," << col;
    }

    size_t n_rows = par->simulation_duration;
    std::vector<std::string> inserts(n_rows);
    std::stringstream sql;
    for (size_t t = 0; t < n_rows; ++t) {
        sql << "INSERT INTO met VALUES ("
            << par->simulation_serial << ","
            << t << ","
            << ledger->get_cumul_infs(VACCINATED, INFLUENZA, t) << ","
            << ledger->get_cumul_infs(VACCINATED, NON_INFLUENZA, t) << ","
            << ledger->get_cumul_infs(UNVACCINATED, INFLUENZA, t) << ","
            << ledger->get_cumul_infs(UNVACCINATED, NON_INFLUENZA, t) << ","
            << ledger->get_cumul_mais(VACCINATED, INFLUENZA, t) << ","
            << ledger->get_cumul_mais(VACCINATED, NON_INFLUENZA, t) << ","
            << ledger->get_cumul_mais(UNVACCINATED, INFLUENZA, t) << ","
            << ledger->get_cumul_mais(UNVACCINATED, NON_INFLUENZA, t) << ","
            << ledger->get_tnd_ve_est(t) << ");";
        
        inserts[t] = sql.str();
        sql.str(std::string());
    }
    return inserts;
}

void DatabaseHandler::write_metrics(const Ledger* ledger, const Parameters* par) const {
    std::vector<std::string> inserts = prepare_insert_sql(ledger, par);

    for (size_t i = 0; i < n_transaction_attempts; ++i) {
        try {
            SQLite::Database db(database_path, SQLite::OPEN_READWRITE);
            SQLite::Transaction transaction(db);
            for (auto& sql : inserts) {
                SQLite::Statement query(db, sql);
                query.exec();
                query.reset();
            }
            transaction.commit();
            std::cerr << "Write attempt " << i << " succeeded." << '\n';
            break;
        } catch (std::exception& e) {
            std::cerr << "Write attempt " << i << " failed:" << '\n';
            std::cerr << "\tSQLite exception: " << e.what() << '\n';
exit(-2);
            std::this_thread::sleep_for(std::chrono::milliseconds(ms_delay_between_attempts));
        }
    }
}