Parameters = {}

Parameters["parameters"] = {}

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
    value = 1e3,
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
        local ret = (v == 0.5)
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
        local ret = (v == 0.0)
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
        local ret = (v == 0.0)
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
        local ret = (v == 0.0)
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
        local ret = (v == 0.5)
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
        local ret = (v == 1e-1)
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
        local ret = (v == 1.0)
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
        local ret = (v == 0.0)
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
        local ret = (v == 1.0)
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
        local ret = (v == 1e-1)
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
        local ret = (v == 1.0)
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
        local ret = (v == 0.0)
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
        local ret = (v == 1.0)
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
    value = 0.0,
    validate = function(v)
        local ret = (v == 0.0)
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
        local ret = (v == 1.0)
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
        local ret = (v == 0.0)
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
        local ret = (v == 1.0)
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
    value = 0.0,
    validate = function(v)
        local ret = (v == 0.0)
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
        local ret = (v == 1.0)
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
        local ret = (v == 0.0)
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
        local ret = (v >= 0.0) and (v <= 1.0)
        return ret
    end
}

Parameters["parameters"]["influenza_vaccine_effect_distribution_variance"] = {
    nickname = "flu_vax_effect_var",
    description = [[If influenza vaccine efficacy is continuous, then this is the
    variance for a beta distribution of vaccine efficacy for a vaccinated individual.]],
    flag  = "const",
    datatype = "double",
    value = 0.0,
    validate = function(v)
        local ret = (v == 0.0)
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
        local ret = (v == 0.0)
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
        local ret = (v == 0.0)
        return ret
    end
}

Parameters["parameters"]["noninfluenza_vaccine_effect_distribution_variance"] = {
    nickname = "nonflu_vax_effect_var",
    description = [[If noninfluenza vaccine efficacy is continuous, then this is the
    variance for a beta distribution of vaccine efficacy for a vaccinated individual.]],
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
    description = [[The probability than an influenza infection develops symptoms.]],
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
    description = [[The probability than a noninfluenza infection develops symptoms.]],
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
    description = [[The probability than a vaccinated individual will seek medical
    care if they have a symptomatic infection.]],
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
    description = [[The probability than an unvaccinated individual will seek medical
    care if they have a symptomatic infection.]],
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
    description = [[The daily probability that an individual is exposed (ie, can
    have an opportunity for infection) to influenza.]],
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
    description = [[The daily probability that an individual is exposed (ie, can
    have an opportunity for infection) to noninfluenza.]],
    flag  = "const",
    datatype = "double",
    value = 0.001,
    validate = function(v)
        local ret = (v == 0.001)
        return ret
    end
}