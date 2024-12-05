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

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <SQLiteCpp/SQLiteCpp.h>

#include <storyteller/database_handler.hpp>
#include <storyteller/ledger.hpp>
#include <storyteller/storyteller.hpp>
#include <storyteller/tome.hpp>

using namespace std::chrono;

// std::map<std::string, ConfigParFlag> cfg_par_flag_lookup = {
//     {"const", CONST},
//     // {"copy",  COPY},
//     {"step",  STEP}
// };

ParticleJob::ParticleJob() {}

ParticleJob::ParticleJob(size_t serial)
    : serial(serial),
      attempts(0),
      completions(0),
      status("prep"),
      start_time(-1),
      end_time(-1),
      duration(-1) {}

void ParticleJob::start() {
    attempts += 1;
    start_time = duration_cast<seconds>(high_resolution_clock::now().time_since_epoch()).count();
    status = "running";
}

void ParticleJob::end() {
    completions += 1;
    end_time = duration_cast<seconds>(high_resolution_clock::now().time_since_epoch()).count();
    duration = end_time - start_time;
    status = "done";
}

std::string ParticleJob::update() {
    std::ostringstream sql("UPDATE job SET ", std::ios_base::ate);
    sql << "status='"      << status << "', "
        << "attempts="      << attempts << ", "
        << "completions="  << completions << ", "
        << "start_time="   << start_time << ", "
        << "duration="     << duration << " "
        << "WHERE serial=" << serial << ";";
    return sql.str();
}

DatabaseHandler::DatabaseHandler(const Storyteller* storyteller) 
    : n_transaction_attempts(10),
      ms_delay_between_attempts(1000),
      owner(storyteller),
      tome(storyteller->get_tome()) {
    database_path = tome->get_path("database");
}

DatabaseHandler::~DatabaseHandler() {}

void DatabaseHandler::read_job(unsigned int serial) {
    simulation_job = ParticleJob(serial);
    try {
        SQLite::Database db(database_path, SQLite::OPEN_READONLY);
        SQLite::Statement query(db, "SELECT * FROM job WHERE serial = ?");
        query.bind(1, serial);
        while (query.executeStep()) {
            simulation_job.attempts = (unsigned int) query.getColumn("attempts");
            simulation_job.completions = (unsigned int) query.getColumn("completions");
            simulation_job.status = (std::string) query.getColumn("status");
        }

        if (owner->get_flag("verbose")) {
            std::cerr << "Read job " << serial << " succeeded." << '\n';
        } else {
            std::cerr << serial << ": job ";
        }
    } catch (std::exception& e) {
        std::cerr << "Read job " << serial << " failed:" << '\n';
        std::cerr << "\tSQLite exception: " << e.what() << '\n';
    }
}

void DatabaseHandler::start_job(unsigned int serial) {
    read_job(serial);
    simulation_job.start();

    try {
        SQLite::Database db(database_path, SQLite::OPEN_READWRITE);
        SQLite::Transaction transaction(db);
        db.exec(simulation_job.update());
        transaction.commit();

        if (owner->get_flag("verbose")) {
            std::cerr << "Start job " << serial << " succeeded." << '\n';
        } else {
            std::cerr << "started... ";
        }
    } catch (std::exception& e) {
        std::cerr << "Start job " << serial << " failed:" << '\n';
        std::cerr << "\tSQLite exception: " << e.what() << '\n';
    }
}

void DatabaseHandler::end_job(unsigned int serial) {
    simulation_job.end();

    try {
        SQLite::Database db(database_path, SQLite::OPEN_READWRITE);
        SQLite::Transaction transaction(db);
        db.exec(simulation_job.update());
        transaction.commit();

        if (owner->get_flag("verbose")) {
            std::cerr << "End job " << serial << " succeeded." << '\n';
        } else {
            std::cerr << "job end\n";
        }
    } catch (std::exception& e) {
        std::cerr << "End job " << serial << " failed:" << '\n';
        std::cerr << "\tSQLite exception: " << e.what() << '\n';
    }
}

std::map<std::string, double> DatabaseHandler::read_parameters(unsigned int serial, const std::vector<std::string>& pars) {
    std::map<std::string, double> ret;
    for (size_t i = 0; i < n_transaction_attempts; ++i) {
        ret.clear();
        try {
            SQLite::Database db(database_path, SQLite::OPEN_READONLY);
            SQLite::Statement query(db, "SELECT * FROM par WHERE serial = ?");
            query.bind(1, serial);
            while (query.executeStep()) {
                for (auto& nickname : pars) {
                    ret[nickname] = query.getColumn(nickname.c_str());
                }
            }

            if (owner->get_flag("verbose")) {
                std::cerr << "Read attempt " << i << " succeeded." << '\n';
                if (owner->get_flag("very_verbose")) {
                    for (const auto& [k,v] : ret) {
                        std::cerr << k << ": " << v << '\n';
                    }
                }
            } else {
                std::cerr << "params read... ";
            }
            break;
        } catch (std::exception& e) {
            std::cerr << "Read attempt " << i << " failed:" << '\n';
            std::cerr << "\tSQLite exception: " << e.what() << '\n';
            std::this_thread::sleep_for(milliseconds(ms_delay_between_attempts));
        }
    }
    return ret;
}

std::vector<std::string> DatabaseHandler::prepare_insert_sql(const Ledger* ledger, const Parameters* par) const {
    size_t n_rows = par->get("sim_duration");
    std::vector<std::string> inserts(n_rows);
    std::string tmp_col_order = "(serial,time,c_vax_flu_inf,c_vax_nonflu_inf,c_unvax_flu_inf,c_unvax_nonflu_inf,c_vax_flu_mai,c_vax_nonflu_mai,c_unvax_flu_mai,c_unvax_nonflu_mai,tnd_ve_est)";
    std::stringstream sql;
    for (size_t t = 0; t < n_rows; ++t) {
        sql << "INSERT INTO met "
            << tmp_col_order
            << " VALUES ("
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

void DatabaseHandler::write_metrics(const Ledger* ledger, const Parameters* par) {
    std::vector<std::string> inserts = prepare_insert_sql(ledger, par);
    if (simulation_job.completions > 0) clear_metrics(par->simulation_serial);

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

            if (owner->get_flag("verbose")) {
                std::cerr << "Write attempt " << i << " succeeded." << '\n';
            } else {
                std::cerr << "mets written... ";
            }
            break;
        } catch (std::exception& e) {
            std::cerr << "Write attempt " << i << " failed:" << '\n';
            std::cerr << "\tSQLite exception: " << e.what() << '\n';
            std::this_thread::sleep_for(milliseconds(ms_delay_between_attempts));
        }
    }
}

void DatabaseHandler::clear_metrics(unsigned int serial) {
    for (size_t i = 0; i < n_transaction_attempts; ++i) {
        try {
            SQLite::Database db(database_path, SQLite::OPEN_READWRITE);
            SQLite::Transaction transaction(db);
            SQLite::Statement query(db, "DELETE FROM met WHERE serial=?");
            query.bind(1, serial);
            query.exec();
            query.reset();
            transaction.commit();

            if (owner->get_flag("verbose")) {
                std::cerr << "Clear attempt " << serial << " succeeded." << '\n';
            }
            break;
        } catch (std::exception& e) {
            std::cerr << "Clear attempt " << serial << " failed:" << '\n';
            std::cerr << "\tSQLite exception: " << e.what() << '\n';
            std::this_thread::sleep_for(milliseconds(ms_delay_between_attempts));
        }
    }
}

bool DatabaseHandler::database_exists() {
    try {
        SQLite::Database db(database_path, SQLite::OPEN_READONLY);
        return db.tableExists("par") and db.tableExists("met") and db.tableExists("job");
    } catch (std::exception& e) {
        return false;
    }
}

bool DatabaseHandler::table_exists(std::string table) {
    SQLite::Database db(database_path, SQLite::OPEN_READONLY);
    return db.tableExists(table);
}

void DatabaseHandler::drop_table_if_exists(std::string table) {
    try {
        SQLite::Database db(database_path, SQLite::OPEN_READWRITE);
        auto sql = "DROP TABLE IF EXISTS " + table;
        SQLite::Transaction transaction(db);
        db.exec(sql);
        transaction.commit();

        if (owner->get_flag("verbose")) {
            std::cerr << "Drop attempt for " << table << " succeeded." << '\n';
        }
    } catch (std::exception& e) {
        std::cerr << "Drop attempt for " << table << " failed:" << '\n';
        std::cerr << "\tSQLite exception: " << e.what() << '\n';
        std::this_thread::sleep_for(milliseconds(ms_delay_between_attempts));
    }
}

void DatabaseHandler::import_metrics_from(std::string file_path) {
    auto import_str = table_exists("met") ? ".import --csv --skip 1 " : ".import --csv ";
    try {
        std::ostringstream cmd("sqlite3 ", std::ios_base::ate);
        cmd << database_path << R"( ")" << import_str << file_path << R"( met")";
        auto ret = system(cmd.str().c_str());

        if (ret == 0) std::cerr << "Import attempt for " << file_path << " succeeded." << '\n';
    } catch (std::exception& e) {
        std::cerr << "Import attempt for " << file_path << " failed:" << '\n';
        std::cerr << "\t" << e.what() << '\n';
        std::this_thread::sleep_for(milliseconds(ms_delay_between_attempts));
    }
}

int DatabaseHandler::init_database() {
    auto par_table        = tome->get_config_params();
    auto cfg_pars         = par_table.at("parameters").as<sol::table>();
    auto cfg_mets         = tome->get_config_metrics();
    size_t n_realizations = tome->get_element_as<size_t>("n_realizations");
    auto tolerance        = tome->get_element_as<double>("par_value_tolerance");

    // parameter pre-processing
    std::map<std::string, std::vector<std::string>> par_names_by_type;
    std::map<std::string, std::string> par_flags;
    for (const auto& [key, p] : cfg_pars) {
        const auto fullname = key.as<std::string>();
        const auto flag     = p.as<sol::table>().get<std::string>("flag");

        if ((flag == "const") or (flag == "step") or (flag == "copy")) {
            par_flags[fullname] = flag;
            par_names_by_type[flag].push_back(fullname);
        } else {
            std::cerr << "ERROR: " << fullname << " has an unsupported flag (" << flag << ")\n";
            exit(-1);
        }
    }

    // process const, step, copy params in that order
    std::vector<std::string> par_fullnames(par_names_by_type["const"]);
    par_fullnames.reserve(par_names_by_type["const"].size() + par_names_by_type["step"].size() + par_names_by_type["copy"].size());
    par_fullnames.insert(par_fullnames.end(), par_names_by_type["step"].begin(), par_names_by_type["step"].end());
    par_fullnames.insert(par_fullnames.end(), par_names_by_type["copy"].begin(), par_names_by_type["copy"].end());

    std::map<std::string, std::string> par_nicknames;
    std::map<std::string, std::string> par_datatypes;
    std::map<std::string, std::string> par_copy_who;
    std::map<std::string, std::vector<double>> par_values;
    std::map<std::string, std::string> par_name_lookup;
    for (const auto& fullname : par_fullnames) {
        const auto p        = cfg_pars.get<sol::table>(fullname);
        const auto nickname = p.get_or<std::string>("nickname", fullname);
        const auto datatype = p.get<std::string>("datatype");
        const auto flag     = par_flags.at(fullname);

        par_name_lookup[fullname] = fullname;
        par_name_lookup[nickname] = fullname;
        par_nicknames[fullname]   = nickname;
        par_datatypes[fullname]   = datatype;

        if (flag == "const") {
            par_values[fullname] = {p.get<double>("value")};
        } else if (flag == "step") {
            auto defined_vals = p.get<sol::optional<std::vector<double>>>("values");
            if (defined_vals) {
                for (const double& v : defined_vals.value()) {
                    par_values[fullname].push_back(v);
                }
            } else {
                const auto start    = p.get<double>("lower");
                const auto end      = p.get<double>("upper");
                const auto step     = p.get<double>("step");
                const size_t n_vals = std::ceil(((end - start) / step) + 1);

                double v = start;
                for (size_t i = 0; i < n_vals; ++i) {
                    par_values[fullname].push_back(v);
                    v += step;
                }

                if (std::abs((v - step) - end) > tolerance) {
                    std::cerr << "ERROR: non-sensible step size for " << fullname << '\n';
                    std::cerr << std::abs(v - end) << " > " << tolerance << '\n';
                    exit(-1);
                }
            }
        } else if (flag == "copy") {
            const auto who = p.get<std::string>("who");
            const auto par_to_copy = par_name_lookup.at(who);
            const auto flag_to_copy = par_flags.at(par_to_copy);

            if ((flag_to_copy == "const") or (flag_to_copy == "step")) {
                par_copy_who[fullname] = par_to_copy;
            } else {
                std::cerr << "ERROR: " << fullname << " copies " << par_to_copy << " with unsupported flag (" << flag_to_copy << ")\n";
                exit(-1);
            }
        } else {
            std::cerr << "ERROR: " << fullname << " has an unsupported flag (" << flag << ")\n";
            exit(-1);
        }
    }

    // calculate step param combinations or create empty single row if no step params exist
    std::vector<std::string> sql_par_col_order;
    vector2d<double> par_rows;
    if (not par_names_by_type.at("step").empty()) {
        vector2d<double> step_par_vecs;
        for (const auto& fullname : par_names_by_type.at("step")) {
            sql_par_col_order.push_back(par_nicknames.at(fullname));
            step_par_vecs.push_back(par_values.at(fullname));
        }
        par_rows = util::vec_combinations(step_par_vecs);
    } else {
        par_rows = {{}};
    }

    for (const auto& fullname : par_names_by_type.at("const")) {
        const auto val = par_values.at(fullname).front();

        sql_par_col_order.push_back(par_nicknames.at(fullname));
        if (not par_rows.empty()) {
            for (auto& row : par_rows) {
                row.push_back(val);
            }
        } else {
            par_rows.front().push_back(val);
        }
    }

    for (const auto& fullname : par_names_by_type.at("copy")) {
        const auto who_fullname = par_copy_who.at(fullname);
        const auto who_nickname = par_nicknames.at(who_fullname);
        const auto who_idx      = std::find(sql_par_col_order.cbegin(), sql_par_col_order.cend(), who_nickname) - sql_par_col_order.cbegin();

        sql_par_col_order.push_back(par_nicknames.at(fullname));
        for (auto& row : par_rows) {
            row.push_back(row[who_idx]);
        }
    }

    std::vector<std::string> sql;

    std::ostringstream met_table_sql("CREATE TABLE met (serial INT", std::ios_base::ate);
    for (const auto& [name, el] : cfg_mets) {
        sol::table m = el.as<sol::table>();
        met_table_sql << ", " << name << " " << m.get<std::string>("datatype");
    }
    met_table_sql << ");";
    sql.push_back(met_table_sql.str());

    std::ostringstream par_table_sql("CREATE TABLE par (serial INT, seed INT", std::ios_base::ate);
    for (const auto& col : sql_par_col_order) {
        const auto fullname = par_name_lookup.at(col);
        const auto datatype = par_datatypes.at(fullname);
        par_table_sql << ", " << col << " " << datatype;
    }

    par_table_sql << ");";
    sql.push_back(par_table_sql.str());

    std::string job_table_sql("CREATE TABLE job (serial INT, status TEXT, start_time INT, duration REAL, attempts INT, completions INT)");
    sql.push_back(job_table_sql);

    std::string job_insert_leader("INSERT INTO job VALUES (");
    std::string job_insert_trailer(", 'queued', -1, -1, 0, 0);");
    std::ostringstream job_insert_sql(job_insert_leader, std::ios_base::ate);

    std::ostringstream par_insert_leader("INSERT INTO par (serial, seed", std::ios_base::ate);
    for (const auto& col : sql_par_col_order) {
        par_insert_leader << ", " << col;
    }
    par_insert_leader << ") VALUES (";

    std::ostringstream par_insert_sql(par_insert_leader.str(), std::ios_base::ate);
    size_t serial = 0, seed = 0;
    for (size_t r = 0; r < n_realizations; ++r) {
        for (const auto& row : par_rows) {
            job_insert_sql << serial << job_insert_trailer;
            par_insert_sql << serial << ", " << seed;
            for (const auto& val : row) {
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