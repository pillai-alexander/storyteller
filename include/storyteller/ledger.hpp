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

    vector3d<size_t> get_cumul_infs() const;
    vector3d<size_t> get_cumul_sympt_infs() const;
    vector3d<size_t> get_cumul_mais() const;

    std::vector<size_t> get_vax_incidence() const;
    std::vector<double> get_tnd_ve_est() const;

    size_t get_cumul_infs(VaccinationStatus vaxd, StrainType strain, size_t time) const;
    size_t get_cumul_sympt_infs(VaccinationStatus vaxd, StrainType strain, size_t time) const;
    size_t get_cumul_mais(VaccinationStatus vaxd, StrainType strain, size_t time) const;
    double get_tnd_ve_est(size_t time) const;

    void log_infection(const Infection* i);

    size_t total_infections(VaccinationStatus vaxd, StrainType strain) const;
    size_t total_sympt_infections(VaccinationStatus vaxd, StrainType strain) const;
    size_t total_mai(VaccinationStatus vaxd, StrainType strain) const;
    size_t total_vaccinations() const;

    void calculate_cumulatives();
    void calculate_tnd_ve_est();

    void generate_linelist_csv(std::string filepath = "");
    void generate_simvis_csv(std::string filepath = "");

  private:
    // EPIDEMIC DATA
    std::vector<const Infection*> infections;
    vector3d<size_t> inf_incidence;       // [vax status][strain][time]
    vector3d<size_t> sympt_inf_incidence; // [vax status][strain][time]
    vector3d<size_t> mai_incidence;       // [vax status][strain][time]

    vector3d<size_t> cumul_infs;       // [vax status][strain][time]
    vector3d<size_t> cumul_sympt_infs; // [vax status][strain][time]
    vector3d<size_t> cumul_mais;       // [vax status][strain][time]

    std::vector<double> tnd_ve_estimate; // [time]

    // POPULATION DATA
    std::vector<size_t> vax_incidence; // [time]

    std::string linelist_header;
    std::string simvis_header;

    const Parameters* par;
};