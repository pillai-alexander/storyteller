Parameters = {}

Parameters["parameters"] = {}

-- Parameters["parameters"]["fullname"] = {
--     nickname = "nickname",
--     description = "description",
--     flag  = "const",
--     datatype = "integer/double",
--     value = value,
--     validate = function(v)
--         local ret = (boolean check)
--         return ret
--     end
-- }

-- Parameters["parameters"]["fullname"] = {
--     nickname = "nickname",
--     description = "description",
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

-- Parameters["parameters"]["fullname"] = {
--     nickname = "nickname",
--     description = "description",
--     flag  = "step",
--     datatype = "integer/double",
--     values = {list of values}
--     validate = function(v)
--         local ret = (boolean check)
--         return ret
--     end
-- }

-- Parameters["parameters"]["fullname"] = {
--     nickname = "nickname",
--     description = "description",
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
    description = "",
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
    description = "",
    flag  = "const",
    datatype = "integer",
    value = 1e3,
    validate = function(v)
        local ret = (v > 0)
        return ret
    end
}

-- PROBABILITY OF IMMUNE STATUSES
Parameters["parameters"]["probability_of_vaccination"] = {
    nickname = "pr_vax",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 0.5,
    validate = function(v)
        local ret = (v == 0.5)
        return ret
    end
}

Parameters["parameters"]["probability_of_prior_immunity_if_vaccinated"] = {
    nickname = "pr_prior_imm_vaxd",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v == 0.0)
        return ret
    end
}

Parameters["parameters"]["probability_of_prior_immunity_if_unvaccinated"] = {
    nickname = "pr_prior_imm_unvaxd",
    description = "",
    flag  = "copy",
    datatype = "double",
    who = "pr_prior_imm_vaxd",
    validate = function(v)
        local ret = (v == 0.0)
        return ret
    end
}

-- VACCINATED INFLUENZA SUSCEPTIBILITY PARAMETERS
Parameters["parameters"]["vaccinated_influenza_susceptibility_distribution_is_continuous"] = {
    nickname = "vaxd_flu_suscep_is_contin",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v == 0.0)
        return ret
    end
}

Parameters["parameters"]["vaccinated_influenza_susceptibility_distribution_mean"] = {
    nickname = "vaxd_flu_suscep_mean",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 0.5,
    validate = function(v)
        local ret = (v == 0.5)
        return ret
    end
}

Parameters["parameters"]["vaccinated_influenza_susceptibility_distribution_standard_deviation"] = {
    nickname = "vaxd_flu_suscep_sd",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 1e-1,
    validate = function(v)
        local ret = (v == 1e-1)
        return ret
    end
}

Parameters["parameters"]["vaccinated_influenza_susceptibility_baseline"] = {
    nickname = "vaxd_flu_suscep_baseline",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v == 1.0)
        return ret
    end
}

-- UNVACCINATED INFLUENZA SUSCEPTIBILITY PARAMETERS
Parameters["parameters"]["unvaccinated_influenza_susceptibility_distribution_is_continuous"] = {
    nickname = "unvaxd_flu_suscep_is_contin",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v == 0.0)
        return ret
    end
}

Parameters["parameters"]["unvaccinated_influenza_susceptibility_distribution_mean"] = {
    nickname = "unvaxd_flu_suscep_mean",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v == 1.0)
        return ret
    end
}

Parameters["parameters"]["unvaccinated_influenza_susceptibility_distribution_standard_deviation"] = {
    nickname = "unvaxd_flu_suscep_sd",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 1e-1,
    validate = function(v)
        local ret = (v == 1e-1)
        return ret
    end
}

Parameters["parameters"]["unvaccinated_influenza_susceptibility_baseline"] = {
    nickname = "unvaxd_flu_suscep_baseline",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v == 1.0)
        return ret
    end
}

-- VACCINATED NONINFLUENZA SUSCEPTIBILITY PARAMETERS
Parameters["parameters"]["vaccinated_noninfluenza_susceptibility_distribution_is_continuous"] = {
    nickname = "vaxd_nonflu_suscep_is_contin",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v == 0.0)
        return ret
    end
}

Parameters["parameters"]["vaccinated_noninfluenza_susceptibility_distribution_mean"] = {
    nickname = "vaxd_nonflu_suscep_mean",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v == 1.0)
        return ret
    end
}

Parameters["parameters"]["vaccinated_noninfluenza_susceptibility_distribution_standard_deviation"] = {
    nickname = "vaxd_nonflu_suscep_sd",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v == 0.0)
        return ret
    end
}

Parameters["parameters"]["vaccinated_noninfluenza_susceptibility_baseline"] = {
    nickname = "vaxd_nonflu_suscep_baseline",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v == 1.0)
        return ret
    end
}

-- UNVACCINATED NONINFLUENZA SUSCEPTIBILITY PARAMETERS
Parameters["parameters"]["unvaccinated_noninfluenza_susceptibility_distribution_is_continuous"] = {
    nickname = "unvaxd_nonflu_suscep_is_contin",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v == 0.0)
        return ret
    end
}

Parameters["parameters"]["unvaccinated_noninfluenza_susceptibility_distribution_mean"] = {
    nickname = "unvaxd_nonflu_suscep_mean",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v == 1.0)
        return ret
    end
}

Parameters["parameters"]["unvaccinated_noninfluenza_susceptibility_distribution_standard_deviation"] = {
    nickname = "unvaxd_nonflu_suscep_sd",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v == 0.0)
        return ret
    end
}

Parameters["parameters"]["unvaccinated_noninfluenza_susceptibility_baseline"] = {
    nickname = "unvaxd_nonflu_suscep_baseline",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v == 1.0)
        return ret
    end
}

-- INFLUENZA VACCINE PARAMETERS
Parameters["parameters"]["influenza_vaccine_effect_distribution_is_continuous"] = {
    nickname = "flu_vax_effect_is_contin",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v == 0.0)
        return ret
    end
}

Parameters["parameters"]["influenza_vaccine_effect_distribution_mean"] = {
    nickname = "flu_vax_effect_mean",
    description = "",
    flag  = "step",
    datatype = "double",
    lower = 0.0,
    upper = 1.0,
    step = 0.1,
    validate = function(v)
        local ret = (v >= 0.0) and (v <= 1.0)
        return ret
    end
}

Parameters["parameters"]["influenza_vaccine_effect_distribution_variance"] = {
    nickname = "flu_vax_effect_var",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v == 0.0)
        return ret
    end
}

Parameters["parameters"]["influenza_vaccine_waning_protection"] = {
  nickname = "flu_vax_effect_wanes",
  description = "",
  flag  = "const",
  datatype = "double",
  value = 0.0,
  validate = function(v)
      local ret = (v == 0.0) or (v == 1.0)
      return ret
  end
}

Parameters["parameters"]["influenza_vaccine_half_life"] = {
  nickname = "flu_vax_effect_half_life",
  description = "",
  flag  = "const",
  datatype = "double",
  value = 200,
  validate = function(v)
      local ret = (v > 0)
      return ret
  end
}

-- NONINFLUENZA VACCINE PARAMETERS
Parameters["parameters"]["noninfluenza_vaccine_effect_distribution_is_continuous"] = {
    nickname = "nonflu_vax_effect_is_contin",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v == 0.0)
        return ret
    end
}

Parameters["parameters"]["noninfluenza_vaccine_effect_distribution_mean"] = {
    nickname = "nonflu_vax_effect_mean",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v == 0.0)
        return ret
    end
}

Parameters["parameters"]["noninfluenza_vaccine_effect_distribution_variance"] = {
    nickname = "nonflu_vax_effect_var",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v == 0.0)
        return ret
    end
}

-- INFECTION OUTCOME PARAMETERS
Parameters["parameters"]["probability_of_symptoms_if_influenza"] = {
    nickname = "pr_sympt_flu",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v == 1.0)
        return ret
    end
}

Parameters["parameters"]["probability_of_symptoms_if_noninfluenza"] = {
    nickname = "pr_sympt_nonflu",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v == 1.0)
        return ret
    end
}

Parameters["parameters"]["probability_of_care_seeking_if_vaccinated"] = {
    nickname = "pr_careseeking_vaxd",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v == 1.0)
        return ret
    end
}

Parameters["parameters"]["probability_of_care_seeking_if_unvaccinated"] = {
    nickname = "pr_careseeking_unvaxd",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 1.0,
    validate = function(v)
        local ret = (v == 1.0)
        return ret
    end
}

-- EXPOSURE/FORCE OF INFECTION PARAMETERS
Parameters["parameters"]["probability_of_daily_influenza_exposure"] = {
    nickname = "pr_flu_exposure",
    description = "",
    flag  = "step",
    datatype = "double",
    values = {0.001, 0.005, 0.01},
    validate = function(v)
        local ret = (v >= 0.001) and (v <= 0.01)
        return ret
    end
}

Parameters["parameters"]["probability_of_daily_noninfluenza_exposure"] = {
    nickname = "pr_nonflu_exposure",
    description = "",
    flag  = "const",
    datatype = "double",
    value = 0.001,
    validate = function(v)
        local ret = (v == 0.001)
        return ret
    end
}

-- SEASONAL FORCING PARAMETERS
Parameters["parameters"]["seasonal_forcing_amplitude_multiplier"] = {
  nickname = "seasonal_amplitude_mult",
  description = "",
  flag  = "const",
  datatype = "double",
  value = 0.0,
  validate = function(v)
      local ret = (v == 0.0)
      return ret
  end
}

Parameters["parameters"]["seasonal_forcing_period"] = {
  nickname = "seasonal_period",
  description = "",
  flag  = "const",
  datatype = "integer",
  value = 200,
  validate = function(v)
      local ret = (v == 200)
      return ret
  end
}

Parameters["parameters"]["seasonal_forcing_phase_shift"] = {
  nickname = "seasonal_shift",
  description = "",
  flag  = "const",
  datatype = "integer",
  value = 100,
  validate = function(v)
      local ret = (v == 100)
      return ret
  end
}