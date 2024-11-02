/**
 * @file database_handler.cpp
 * @author Alexander N. Pillai
 * @brief Contains the DatabaseHandler class that performs all SQLite database
 *        operations.
 *
 * @copyright TBD
 */
#include <chrono>
#include <thread>
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <string>

#include <SQLiteCpp/SQLiteCpp.h>
#include <nlohmann/json.hpp>

#include <storyteller/database_handler.hpp>
#include <storyteller/ledger.hpp>

using json = nlohmann::json;
using namespace std::chrono;

std::map<std::string, ConfigParFlag> cfg_par_flag_lookup = {
    {"CONST", CONST},
    {"COPY", COPY},
    {"STEP", STEP}
};

DatabaseHandler::DatabaseHandler(const Storyteller* storyteller, std::string db_path) 
    : n_transaction_attempts(10),
      ms_delay_between_attempts(1000) {
    // owner = storyteller;
    database_path = db_path;
}

DatabaseHandler::~DatabaseHandler() {}

void DatabaseHandler::start_job(unsigned int serial) {
    size_t start = duration_cast<seconds>(high_resolution_clock::now().time_since_epoch()).count();
    std::ostringstream job_insert_sql("UPDATE job SET status='R', start_time=", std::ios_base::ate);
    job_insert_sql << start << ", attempts=";
    unsigned int attempts = 1;

    try {
        SQLite::Database db(database_path, SQLite::OPEN_READWRITE);
        SQLite::Statement query(db, "SELECT * FROM job WHERE serial = ?");
        query.bind(1, serial);
        while (query.executeStep()) {
            attempts += (unsigned int) query.getColumn("attempts");
        }

        job_insert_sql << attempts << " WHERE serial=" << serial << ";";

        SQLite::Transaction transaction(db);
        db.exec(job_insert_sql.str());
        transaction.commit();
        std::cerr << "Start job " << serial << " succeeded." << '\n';
    } catch (std::exception& e) {
        std::cerr << "Start job " << serial << " failed:" << '\n';
        std::cerr << "\tSQLite exception: " << e.what() << '\n';
    }
}

void DatabaseHandler::read_parameters(unsigned int serial, std::map<std::string, double>& pars) {
    for (size_t i = 0; i < n_transaction_attempts; ++i) {
        start_job(serial);
        try {
            SQLite::Database db(database_path);
            SQLite::Statement query(db, "SELECT * FROM par WHERE serial = ?");
            query.bind(1, serial);
            while (query.executeStep()) {
                pars["seed"] = query.getColumn("seed");
                for (auto& [param, val] : pars) {
                    pars[param] = query.getColumn(param.c_str());
                }
            }
            std::cerr << "Read attempt " << i << " succeeded." << '\n';
            break;
        } catch (std::exception& e) {
            std::cerr << "Read attempt " << i << " failed:" << '\n';
            std::cerr << "\tSQLite exception: " << e.what() << '\n';
            std::this_thread::sleep_for(milliseconds(ms_delay_between_attempts));
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
            std::this_thread::sleep_for(milliseconds(ms_delay_between_attempts));
        }
    }
}

bool DatabaseHandler::database_exists() {
    try {
        SQLite::Database db(database_path, SQLite::OPEN_READONLY);
        return db.tableExists("par") and db.tableExists("met");
    } catch (std::exception& e) {
        return false;
    }
}

bool DatabaseHandler::table_exists(std::string table) {
    SQLite::Database db(database_path, SQLite::OPEN_READONLY);
    return db.tableExists(table);
}

int DatabaseHandler::init_database(json cfg) {
    auto cfg_pars         = cfg["model_parameters"];
    auto cfg_mets         = cfg["metrics"];
    size_t n_realizations = cfg["n_realizations"];

    std::vector<std::string> col_name;
    vector2d<double> step_pars;

    std::vector<std::vector<std::string>> pars_by_flag(NUM_CONFIG_PAR_FLAGS);
    std::map<std::string, std::string> par_types;
    std::map<std::string, std::vector<double>> par_vals;
    std::map<std::string, std::string> copy_who;

    for (auto& [k, el] : cfg_pars.items()) {
        std::string name = el["fullname"].get<std::string>();
        par_types[name] = el["datatype"].get<std::string>();

        auto flag = cfg_par_flag_lookup[el["flag"].get<std::string>()];
        pars_by_flag[flag].push_back(name);
        switch (flag) {
            case CONST: {
                par_vals[name] = std::vector<double>{el["par1"].get<double>()};
                break;
            }
            case STEP: {
                par_vals[name] = std::vector<double>();

                auto start = el["par1"].get<double>();
                auto end = el["par2"].get<double>();
                auto step = el["par3"].get<double>();
                auto n_vals = (end - start) / step;

                if (n_vals != (int) n_vals) {
                    std::cerr << "ERROR: " << name << " has invalid step size.\n";
                    exit(-1);
                }

                for (double v = start; v <= end; v += step) {
                    par_vals[name].push_back(v);
                }
                col_name.push_back(name);
                step_pars.push_back(par_vals[name]);
                break;
            }
            case COPY: {
                par_vals[name] = std::vector<double>();
                copy_who[name] = el["par1"].get<std::string>();
            }
            default: { break; }
        }
    }

    vector2d<double> rows = util::vec_combinations(step_pars);

    for (auto& k : pars_by_flag[CONST]) {
        col_name.push_back(k);
        for (auto& row : rows) {
            row.push_back(par_vals[k].front());
        }
    }

    for (auto& k : pars_by_flag[COPY]) {
        auto copy_from_idx = std::find(col_name.cbegin(), col_name.cend(), copy_who[k]) - col_name.cbegin();
        col_name.push_back(k);
        for (auto& row : rows) {
            row.push_back(row[copy_from_idx]);
        }
    }

    std::vector<std::string> sql;

    std::ostringstream met_table_sql("CREATE TABLE met (serial INT", std::ios_base::ate);
    for (auto& [k, el] : cfg_mets.items()) {
        met_table_sql << ", " << el["fullname"].get<std::string>() << " " << el["datatype"].get<std::string>();
    }
    met_table_sql << ");";
    sql.push_back(met_table_sql.str());

    std::ostringstream par_table_sql("CREATE TABLE par (serial INT, seed INT", std::ios_base::ate);
    for (auto& [k, t] : par_types) {
        par_table_sql << ", " << k << " " << t;
    }
    par_table_sql << ");";
    sql.push_back(par_table_sql.str());

    std::string job_table_sql("CREATE TABLE job (serial INT, status TEXT, start_time INT, duration REAL, attempts INT, completions INT)");
    sql.push_back(job_table_sql);

    std::string job_insert_leader("INSERT INTO job VALUES (");
    std::string job_insert_trailer(", 'Q', -1, -1, 0, 0);");
    std::ostringstream job_insert_sql(job_insert_leader, std::ios_base::ate);

    std::ostringstream par_insert_leader("INSERT INTO par (serial, seed", std::ios_base::ate);
    for (auto& c : col_name) {
        par_insert_leader << "," << c;
    }
    par_insert_leader << ") VALUES (";

    std::ostringstream par_insert_sql(par_insert_leader.str(), std::ios_base::ate);
    size_t serial = 0, seed = 0;
    for (size_t r = 0; r < n_realizations; ++r) {
        for (auto& row : rows) {
            job_insert_sql << serial << job_insert_trailer;
            par_insert_sql << serial << ", " << seed;
            for (auto& val : row) {
                par_insert_sql << ", "<< val;
            }
            par_insert_sql << ");";

            sql.push_back(job_insert_sql.str());
            sql.push_back(par_insert_sql.str());

            job_insert_sql.str(job_insert_leader);
            par_insert_sql.str(par_insert_leader.str());
            ++serial;
        }
        ++seed;
    }

    try {
        SQLite::Database db(database_path, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        SQLite::Transaction transaction(db);
        for (size_t i = 0; i < sql.size(); ++i) {
            SQLite::Statement query(db, sql[i]);
            query.exec();
            query.reset();
        }
        transaction.commit();
        std::cerr << "Database init succeeded." << '\n';
        return 0;
    } catch (std::exception& e) {
        std::cerr << "Database init failed:" << '\n';
        std::cerr << "\tSQLite exception: " << e.what() << '\n';
        return -1;
    }
}