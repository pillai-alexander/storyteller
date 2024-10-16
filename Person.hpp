#pragma once

#include <vector>
#include <memory>
#include "Parameters.hpp"

class Infection {
  public:
    Infection();
    ~Infection();

  private:
    StrainType strain;
    unsigned int infection_time;
    SymptomClass symptoms;
    bool sought_care;
};

class Person {
  public:
    Person();
    ~Person();

    void infect();
    void vaccinate();

  private:
    double susceptibility;
    std::vector<Infection> infection_history;
    VaccinationStatus vaccination_status;
    std::shared_ptr<Parameters> par;
};