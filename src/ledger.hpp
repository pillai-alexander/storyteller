#pragma once

#include <vector>
#include <string>

#include "utility.hpp"
#include "parameters.hpp"

class Infection;

class Ledger {
  friend class Community;
  public:
    Ledger(const Parameters* parameters);
    ~Ledger();

    vector3d<size_t> get_inf_incidence() const;
    vector3d<size_t> get_sympt_inf_incidence() const;
    vector3d<size_t> get_mai_incidence() const;
    std::vector<size_t> get_vax_incidence() const;

    void log_infection(const Infection* i);

    size_t total_infections(VaccinationStatus vaxd, StrainType strain) const;
    size_t total_sympt_infections(VaccinationStatus vaxd, StrainType strain) const;
    size_t total_mai(VaccinationStatus vaxd, StrainType strain) const;
    size_t total_vaccinations() const;

    void generate_linelist_csv(std::string filepath = "");

  private:
    // EPIDEMIC DATA
    std::vector<const Infection*> infections;
    vector3d<size_t> inf_incidence;       // [vax status][strain][time]
    vector3d<size_t> sympt_inf_incidence; // [vax status][strain][time]
    vector3d<size_t> mai_incidence;       // [vax status][strain][time]

    // POPULATION DATA
    std::vector<size_t> vax_incidence; // [time]

    std::string linelist_header;

    const Parameters* par;
};