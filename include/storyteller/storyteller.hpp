#pragma once

#include <map>
#include <string>
#include <memory>

#include <argh.h>

class Simulator;

enum OperationType {
    PROCESS_CONFIG,
    EXAMPLE_SIM,
    BATCH_SIM,
    NUM_OPERATION_TYPES
};

class Storyteller {
  public:
    Storyteller(int argc, char* argv[]);
    ~Storyteller();

    int get_serial() const;
    size_t get_batch_size() const;
    std::string get_config_file() const;

    bool get_flag(std::string key) const;
    void set_flag(std::string key, bool val);

    int run();
  private:
    bool sensible_inputs() const;

    int process_config();
    int default_simulation();
    int batch_simulation();

    int draw_simvis();

    std::unique_ptr<Simulator> simulator;
    OperationType operation_to_perform;

    argh::parser cmdl_args;
    std::map<std::string, bool> simulation_flags;
    int simulation_serial;
    size_t batch_size;
    std::string config_file;
};