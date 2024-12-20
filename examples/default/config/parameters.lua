Parameters = {}

Parameters["parameters"] = {}

-- CONSTANT PARAMETER TEMPLATE
-- Parameters["parameters"]["fullname"] = {
--     nickname = "nickname",
--     description = [[description]],
--     flag  = "const",
--     datatype = "integer/double",
--     value = value,
--     validate = function(v)
--         local ret = (boolean check)
--         return ret
--     end
-- }

-- STEP PARAMETER TEMPLATE
--   DEFINE LOWER AND UPPER ENDPOINTS AND A STEP SIZE
-- Parameters["parameters"]["fullname"] = {
--     nickname = "nickname",
--     description = [[description]],
--     flag  = "step",
--     datatype = "integer/double",
--     lower = lower bound (inclusive),
--     upper = upper bound (inclusive),
--     step = step size,
--     validate = function(v)
--         local ret = (boolean check)
--         return ret
--     end
-- }

-- STEP PARAMETER TEMPLATE
--   DEFINE LIST OF VALUES TO USE
-- Parameters["parameters"]["fullname"] = {
--     nickname = "nickname",
--     description = [[description]],
--     flag  = "step",
--     datatype = "integer/double",
--     values = {list of values}
--     validate = function(v)
--         local ret = (boolean check)
--         return ret
--     end
-- }

-- COPY PARAMETER TEMPLATE
-- Parameters["parameters"]["fullname"] = {
--     nickname = "nickname",
--     description = [[description]],
--     flag  = "copy",
--     datatype = "integer/double",
--     who = "fullname/nickname of param to copy from",
--     validate = function(v)
--         local ret = (boolean check)
--         return ret
--     end
-- }

-- GENERIC PARAMETERS
Parameters["parameters"]["simulation_durations"] = {
    nickname = "sim_duration",
    description = [[The length of a single simulation (in days).]],
    flag  = "const",
    datatype = "integer",
    value = 200,
    validate = function(v)
        local ret = (v > 0)
        return ret
    end
}

Parameters["parameters"]["population_size"] = {
    nickname = "pop_size",
    description = [[The total synthetic population size.]],
    flag  = "const",
    datatype = "integer",
    value = 1e4,
    validate = function(v)
        local ret = (v > 0)
        return ret
    end
}

-- PROBABILITY OF IMMUNE STATUSES
Parameters["parameters"]["probability_of_vaccination"] = {
    nickname = "pr_vax",
    description = [[The probability that an individual will be vaccinated.]],
    flag  = "const",
    datatype = "double",
    value = 0.5,
    validate = function(v)
        local ret = (v >= 0) and (v <= 1)
        return ret
    end
}

Parameters["parameters"]["probability_of_prior_immunity_if_vaccinated"] = {
    nickname = "pr_prior_imm_vaxd",
    description = [[If prior immunity is discrete, this is the probability that
    a vaccinated individual will have prior immunity.]],
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v >= 0) and (v <= 1)
        return ret
    end
}

Parameters["parameters"]["probability_of_prior_immunity_if_unvaccinated"] = {
    nickname = "pr_prior_imm_unvaxd",
    description = [[If prior immunity is discrete, this is the probability that
    an unvaccinated individual will have prior immunity.]],
    flag  = "copy",
    datatype = "double",
    who = "pr_prior_imm_vaxd",
    validate = function(v)
        local ret = (v >= 0) and (v <= 1)
        return ret
    end
}

-- VACCINATED INFLUENZA SUSCEPTIBILITY PARAMETERS
Parameters["parameters"]["vaccinated_influenza_susceptibility_distribution_is_continuous"] = {
    nickname = "vaxd_flu_suscep_is_contin",
    description = [[This boolean flag controls whether susceptibility against
    influenza in the vaccinated population should be treated as continuous (1)
    or discrete (0).]],
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v == 0) or (v == 1)
        return ret
    end
}

Parameters["parameters"]["vaccinated_influenza_susceptibility_distribution_mean"] = {
    nickname = "vaxd_flu_suscep_mean",
    description = [[If susceptibility against influenza in the vaccinated population
    is discrete, then this is the susceptibility for an individual with prior immunity.
    If susceptibility against influenza in the vaccinated population is continuous,
    this is the mean for a normal distribution of the log-odds of susceptibility.]],
    flag  = "const",
    datatype = "double",
    value = 0.5,
    validate = function(v)
        local ret = (v >= 0) and (v <= 1)
        return ret
    end
}

Parameters["parameters"]["vaccinated_influenza_susceptibility_distribution_standard_deviation"] = {
    nickname = "vaxd_flu_suscep_sd",
    description = [[If susceptibility against influenza in the vaccinated population
    is continuous, this is the standard deviation for a normal distribution of the
    log-odds of susceptibility.]],
    flag  = "const",
    datatype = "double",
    value = 1e-1,
    validate = function(v)
        local ret = (v > 0)
        return ret
    end
}

Parameters["parameters"]["vaccinated_influenza_susceptibility_baseline"] = {
    nickname = "vaxd_flu_suscep_baseline",
    description = [[If susceptibility against influenza in the vaccinated population
    is discrete, then this is the susceptibility for an individual without prior
    immunity.]],
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v >= 0) and (v <= 1)
        return ret
    end
}

-- UNVACCINATED INFLUENZA SUSCEPTIBILITY PARAMETERS
Parameters["parameters"]["unvaccinated_influenza_susceptibility_distribution_is_continuous"] = {
    nickname = "unvaxd_flu_suscep_is_contin",
    description = [[This boolean flag controls whether susceptibility against
    influenza in the unvaccinated population should be treated as continuous (1)
    or discrete (0).]],
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v == 0) or (v == 1)
        return ret
    end
}

Parameters["parameters"]["unvaccinated_influenza_susceptibility_distribution_mean"] = {
    nickname = "unvaxd_flu_suscep_mean",
    description = [[If susceptibility against influenza in the unvaccinated population
    is discrete, then this is the susceptibility for an individual with prior immunity.
    If susceptibility against influenza in the unvaccinated population is continuous,
    this is the mean for a normal distribution of the log-odds of susceptibility.]],
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v >= 0) and (v <= 1)
        return ret
    end
}

Parameters["parameters"]["unvaccinated_influenza_susceptibility_distribution_standard_deviation"] = {
    nickname = "unvaxd_flu_suscep_sd",
    description = [[If susceptibility against influenza in the unvaccinated population
    is continuous, this is the standard deviation for a normal distribution of the
    log-odds of susceptibility.]],
    flag  = "const",
    datatype = "double",
    value = 1e-1,
    validate = function(v)
        local ret = (v > 0)
        return ret
    end
}

Parameters["parameters"]["unvaccinated_influenza_susceptibility_baseline"] = {
    nickname = "unvaxd_flu_suscep_baseline",
    description = [[If susceptibility against influenza in the unvaccinated population
    is discrete, then this is the susceptibility for an individual without prior
    immunity.]],
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v >= 0) and (v <= 1)
        return ret
    end
}

-- INFLUENZA INFECTION PARAMETERS
Parameters["parameters"]["influenza_infection_refractory_period_length"] = {
  nickname = "flu_inf_refract_len",
  description = [[The number of days after an influenza infection during which
  the individual cannot be infected by any other pathogen.]],
  flag  = "const",
  datatype = "integer",
  value = 0,
  validate = function(v)
      local ret = (v >= 0)
      return ret
  end
}

Parameters["parameters"]["influenza_infection_generates_immunity"] = {
  nickname = "flu_inf_gen_immunity",
  description = [[This boolean flag controls whether influenza infections generate
  immunity. If immunity is generated (1), after the refractory period, immune
  protection is set to 1 (or susceptibility = 0) conferring complete protection.
  If immunity is not generated (0), after the refractory period, susceptibility
  remains equal to its prior value for that individual.]],
  flag  = "const",
  datatype = "integer",
  value = 1,
  validate = function(v)
      local ret = (v == 0) or (v == 1)
      return ret
  end
}

Parameters["parameters"]["influenza_infection_immunity_wanes"] = {
  nickname = "flu_inf_immunity_wanes",
  description = [[This boolean flag controls whether influenza-infection-derived
  immunity wanes over time after the refractory period (1) or is constant over time
  (0).]],
  flag  = "const",
  datatype = "integer",
  value = 0,
  validate = function(v)
      local ret = (v == 0) or (v == 1)
      return ret
  end
}

Parameters["parameters"]["influenza_infection_immunity_half_life"] = {
  nickname = "flu_inf_immunity_half_life",
  description = [[If influenza-infection-derived immunity wanes over time, the waning
  rate will be derived to produce this specified half-life.]],
  flag  = "const",
  datatype = "integer",
  value = 1500,
  values = {15, 1500},
  validate = function(v)
      local ret = (v > 0)
      return ret
  end
}

-- VACCINATED NONINFLUENZA SUSCEPTIBILITY PARAMETERS
Parameters["parameters"]["vaccinated_noninfluenza_susceptibility_distribution_is_continuous"] = {
    nickname = "vaxd_nonflu_suscep_is_contin",
    description = [[This boolean flag controls whether susceptibility against
    noninfluenza in the vaccinated population should be treated as continuous (1)
    or discrete (0).]],
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v == 0) or (v == 1)
        return ret
    end
}

Parameters["parameters"]["vaccinated_noninfluenza_susceptibility_distribution_mean"] = {
    nickname = "vaxd_nonflu_suscep_mean",
    description = [[If susceptibility against noninfluenza in the vaccinated population
    is discrete, then this is the susceptibility for an individual with prior immunity.
    If susceptibility against noninfluenza in the vaccinated population is continuous,
    this is the mean for a normal distribution of the log-odds of susceptibility.]],
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v >= 0) and (v <= 1)
        return ret
    end
}

Parameters["parameters"]["vaccinated_noninfluenza_susceptibility_distribution_standard_deviation"] = {
    nickname = "vaxd_nonflu_suscep_sd",
    description = [[If susceptibility against noninfluenza in the vaccinated population
    is continuous, this is the standard deviation for a normal distribution of the
    log-odds of susceptibility.]],
    flag  = "const",
    datatype = "double",
    value = 1e-1,
    validate = function(v)
        local ret = (v > 0)
        return ret
    end
}

Parameters["parameters"]["vaccinated_noninfluenza_susceptibility_baseline"] = {
    nickname = "vaxd_nonflu_suscep_baseline",
    description = [[If susceptibility against noninfluenza in the vaccinated population
    is discrete, then this is the susceptibility for an individual without prior
    immunity.]],
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v >= 0) and (v <= 1)
        return ret
    end
}

-- UNVACCINATED NONINFLUENZA SUSCEPTIBILITY PARAMETERS
Parameters["parameters"]["unvaccinated_noninfluenza_susceptibility_distribution_is_continuous"] = {
    nickname = "unvaxd_nonflu_suscep_is_contin",
    description = [[This boolean flag controls whether susceptibility against
    noninfluenza in the unvaccinated population should be treated as continuous (1)
    or discrete (0).]],
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v == 0) or (v == 1)
        return ret
    end
}

Parameters["parameters"]["unvaccinated_noninfluenza_susceptibility_distribution_mean"] = {
    nickname = "unvaxd_nonflu_suscep_mean",
    description = [[If susceptibility against noninfluenza in the unvaccinated population
    is discrete, then this is the susceptibility for an individual with prior immunity.
    If susceptibility against noninfluenza in the unvaccinated population is continuous,
    this is the mean for a normal distribution of the log-odds of susceptibility.]],
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v >= 0) and (v <= 1)
        return ret
    end
}

Parameters["parameters"]["unvaccinated_noninfluenza_susceptibility_distribution_standard_deviation"] = {
    nickname = "unvaxd_nonflu_suscep_sd",
    description = [[If susceptibility against noninfluenza in the unvaccinated population
    is continuous, this is the standard deviation for a normal distribution of the
    log-odds of susceptibility.]],
    flag  = "const",
    datatype = "double",
    value = 1e-1,
    validate = function(v)
        local ret = (v > 0)
        return ret
    end
}

Parameters["parameters"]["unvaccinated_noninfluenza_susceptibility_baseline"] = {
    nickname = "unvaxd_nonflu_suscep_baseline",
    description = [[If susceptibility against noninfluenza in the unvaccinated
    population is discrete, then this is the susceptibility for an individual
    without prior immunity.]],
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v >= 0) and (v <= 1)
        return ret
    end
}

-- NONINFLUENZA INFECTION PARAMETERS
Parameters["parameters"]["noninfluenza_infection_refractory_period_length"] = {
  nickname = "nonflu_inf_refract_len",
  description = [[The number of days after a noninfluenza infection during which
  the individual cannot be infected by any other pathogen.]],
  flag  = "copy",
  datatype = "integer",
  who = "flu_inf_refract_len",
  validate = function(v)
      local ret = (v >= 0)
      return ret
  end
}

Parameters["parameters"]["noninfluenza_infection_generates_immunity"] = {
  nickname = "nonflu_inf_gen_immunity",
  description = [[This boolean flag controls whether noninfluenza infections generate
  immunity. If immunity is generated (1), after the refractory period, immune
  protection is set to 1 (or susceptibility = 0) conferring complete protection.
  If immunity is not generated (0), after the refractory period, susceptibility
  remains equal to its prior value for that individual.]],
  flag  = "const",
  datatype = "integer",
  value = 0,
  validate = function(v)
      local ret = (v == 0) or (v == 1)
      return ret
  end
}

Parameters["parameters"]["noninfluenza_infection_immunity_wanes"] = {
  nickname = "nonflu_inf_immunity_wanes",
  description = [[This boolean flag controls whether noninfluenza-infection-derived
  immunity wanes over time after the refractory period (1) or is constant over time
  (0).]],
  flag  = "const",
  datatype = "integer",
  value = 0,
  validate = function(v)
      local ret = (v == 0) or (v == 1)
      return ret
  end
}

Parameters["parameters"]["noninfluenza_infection_immunity_half_life"] = {
  nickname = "nonflu_inf_immunity_half_life",
  description = [[If noninfluenza-infection-derived immunity wanes over time, the
  waning rate will be derived to produce this specified half-life.]],
  flag  = "const",
  datatype = "integer",
  value = 14,
  validate = function(v)
      local ret = (v > 0)
      return ret
  end
}

-- INFLUENZA VACCINE PARAMETERS
Parameters["parameters"]["influenza_vaccine_effect_distribution_is_continuous"] = {
    nickname = "flu_vax_effect_is_contin",
    description = [[This boolean flag controls whether influenza vaccine efficacy
    (ie, the effect that vaccination has to reduce susceptibility against influenza
    infection) should be treated as continuous (1) or discrete (0).]],
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v == 0) or (v == 1)
        return ret
    end
}

Parameters["parameters"]["influenza_vaccine_effect_distribution_mean"] = {
    nickname = "flu_vax_effect_mean",
    description = [[If influenza vaccine efficacy is discrete, then this is the
    vaccine efficacy for a vaccinated individual. If influenza vaccine efficacy
    is continuous, then this is the mean for a beta distribution of vaccine
    efficacy for a vaccinated individual.]],
    flag  = "step",
    datatype = "double",
    lower = 0.0,
    upper = 1.0,
    step = 0.1,
    validate = function(v)
        local ret = (v >= 0) and (v <= 1)
        return ret
    end
}

Parameters["parameters"]["influenza_vaccine_effect_distribution_variance"] = {
    nickname = "flu_vax_effect_var",
    description = [[If influenza vaccine efficacy is continuous, then this is the
    variance for a beta distribution of vaccine efficacy for a vaccinated individual.]],
    flag  = "const",
    datatype = "double",
    value = 1e-1,
    validate = function(v)
        local ret = (v > 0)
        return ret
    end
}

Parameters["parameters"]["influenza_vaccine_waning_protection"] = {
  nickname = "flu_vax_effect_wanes",
  description = [[This boolean flag controls whether vaccine efficacy against
  influenza infection wanes over time after the refractory period (1) or is constant
  over time (0).]],
  flag  = "const",
  datatype = "double",
  value = 0.0,
  validate = function(v)
      local ret = (v == 0) or (v == 1)
      return ret
  end
}

Parameters["parameters"]["influenza_vaccine_half_life"] = {
  nickname = "flu_vax_effect_half_life",
  description = [[If vaccine efficacy against influenza infection wanes over time,
  the waning rate will be derived to produce this specified half-life.]],
  flag  = "const",
  datatype = "double",
  value = 200,
  validate = function(v)
      local ret = (v > 0)
      return ret
  end
}

Parameters["parameters"]["influenza_vaccine_lag_time_until_waning"] = {
  nickname = "flu_vax_effect_lag_til_waning",
  description = [[If vaccine efficacy against influenza infection wanes over time,
  this controls the lag time between when vaccination occurs and when efficacy
  waning begins.]],
  flag  = "const",
  datatype = "double",
  value = 0,
  validate = function(v)
      local ret = (v >= 0)
      return ret
  end
}

-- NONINFLUENZA VACCINE PARAMETERS
Parameters["parameters"]["noninfluenza_vaccine_effect_distribution_is_continuous"] = {
    nickname = "nonflu_vax_effect_is_contin",
    description = [[This boolean flag controls whether noninfluenza vaccine efficacy
    (ie, the effect that vaccination has to reduce susceptibility against noninfluenza
    infection) should be treated as continuous (1) or discrete (0).]],
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v == 0) or (v == 1)
        return ret
    end
}

Parameters["parameters"]["noninfluenza_vaccine_effect_distribution_mean"] = {
    nickname = "nonflu_vax_effect_mean",
    description = [[If noninfluenza vaccine efficacy is discrete, then this is the
    vaccine efficacy for a vaccinated individual. If noninfluenza vaccine efficacy
    is continuous, then this is the mean for a beta distribution of vaccine
    efficacy for a vaccinated individual.]],
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v >= 0) and (v <= 1)
        return ret
    end
}

Parameters["parameters"]["noninfluenza_vaccine_effect_distribution_variance"] = {
    nickname = "nonflu_vax_effect_var",
    description = [[If noninfluenza vaccine efficacy is continuous, then this is the
    variance for a beta distribution of vaccine efficacy for a vaccinated individual.]],
    flag  = "const",
    datatype = "double",
    value = 1e-1,
    validate = function(v)
        local ret = (v > 0)
        return ret
    end
}

Parameters["parameters"]["noninfluenza_vaccine_waning_protection"] = {
  nickname = "nonflu_vax_effect_wanes",
  description = [[This boolean flag controls whether vaccine efficacy against
  noninfluenza infection wanes over time after the refractory period (1) or is constant
  over time (0).]],
  flag  = "const",
  datatype = "double",
  value = 0.0,
  validate = function(v)
      local ret = (v == 0) or (v == 1)
      return ret
  end
}

Parameters["parameters"]["noninfluenza_vaccine_half_life"] = {
  nickname = "nonflu_vax_effect_half_life",
  description = [[If vaccine efficacy against noninfluenza infection wanes over time,
  the waning rate will be derived to produce this specified half-life.]],
  flag  = "const",
  datatype = "double",
  value = 14,
  validate = function(v)
      local ret = (v > 0)
      return ret
  end
}

Parameters["parameters"]["noninfluenza_vaccine_lag_time_until_waning"] = {
  nickname = "nonflu_vax_effect_lag_til_waning",
  description = [[If vaccine efficacy against noninfluenza infection wanes over time,
  this controls the lag time between when vaccination occurs and when efficacy
  waning begins.]],
  flag  = "const",
  datatype = "double",
  value = 0,
  validate = function(v)
      local ret = (v >= 0)
      return ret
  end
}

-- INFECTION OUTCOME PARAMETERS
Parameters["parameters"]["probability_of_symptoms_if_influenza"] = {
    nickname = "pr_sympt_flu",
    description = [[The probability than an influenza infection develops symptoms.]],
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v >= 0) and (v <= 1)
        return ret
    end
}

Parameters["parameters"]["probability_of_symptoms_if_noninfluenza"] = {
    nickname = "pr_sympt_nonflu",
    description = [[The probability than a noninfluenza infection develops symptoms.]],
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v >= 0) and (v <= 1)
        return ret
    end
}

Parameters["parameters"]["probability_of_care_seeking_if_vaccinated"] = {
    nickname = "pr_careseeking_vaxd",
    description = [[The probability than a vaccinated individual will seek medical
    care if they have a symptomatic infection.]],
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v >= 0) and (v <= 1)
        return ret
    end
}

Parameters["parameters"]["probability_of_care_seeking_if_unvaccinated"] = {
    nickname = "pr_careseeking_unvaxd",
    description = [[The probability than an unvaccinated individual will seek medical
    care if they have a symptomatic infection.]],
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v >= 0) and (v <= 1)
        return ret
    end
}

-- EXPOSURE/FORCE OF INFECTION PARAMETERS
Parameters["parameters"]["probability_of_daily_influenza_exposure"] = {
    nickname = "pr_flu_exposure",
    description = [[The daily probability that an individual is exposed (ie, can
    have an opportunity for infection) to influenza.]],
    flag  = "step",
    datatype = "double",
    values = {0.001, 0.005, 0.01},
    validate = function(v)
        local ret = (v >= 0) and (v <= 1)
        return ret
    end
}

Parameters["parameters"]["probability_of_daily_noninfluenza_exposure"] = {
    nickname = "pr_nonflu_exposure",
    description = [[The daily probability that an individual is exposed (ie, can
    have an opportunity for infection) to noninfluenza.]],
    flag  = "const",
    datatype = "double",
    value = 0.001,
    validate = function(v)
        local ret = (v >= 0) and (v <= 1)
        return ret
    end
}

-- SEASONAL FORCING PARAMETERS
Parameters["parameters"]["seasonal_forcing_amplitude_multiplier"] = {
  nickname = "seasonal_amplitude_mult",
  description = [[This controls how much seasonal forcing is applied to the daily
  exposure probabilities. If it is set to 0, no seasonal forcing is applied and
  daily exposure probabilities are constant over time.]],
  flag  = "const",
  datatype = "double",
  value = 0.0,
  validate = function(v)
      local ret = (v >= 0)
      return ret
  end
}

Parameters["parameters"]["seasonal_forcing_period"] = {
  nickname = "seasonal_period",
  description = [[If daily exposure probabilities seasonally vary over time, this
  controls the period of the seasonal cycling.]],
  flag  = "const",
  datatype = "integer",
  value = 200,
  validate = function(v)
      local ret = (v > 0)
      return ret
  end
}

Parameters["parameters"]["seasonal_forcing_phase_shift"] = {
  nickname = "seasonal_shift",
  description = [[If daily exposure probabilities seasonally vary over time, this
  controls the phase shift of the seasonal cycling.]],
  flag  = "const",
  datatype = "integer",
  value = 100,
  validate = function(v)
      local ret = (v >= 0)
      return ret
  end
}