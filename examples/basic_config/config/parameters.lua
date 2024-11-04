parameters = {}

parameters["vaxd_suscep_distr_shape"] = {
    datatype = "REAL",
    flag = "CONST",
    par1 = 0.0
}

parameters["unvaxd_suscep_distr_shape"] = {
    datatype = "REAL",
    flag = "CONST",
    par1 = 0.0
}

parameters["vaxd_suscep_mean"] = {
    datatype = "REAL",
    flag = "CONST",
    par1 = 1.0
}

parameters["unvaxd_suscep_mean"] = {
    datatype = "REAL",
    flag = "CONST",
    par1 = 1.0
}

parameters["flu_vax_effect_distr_a"] = {
    datatype = "REAL",
    flag = "CONST",
    par1 = 0.0
}

parameters["flu_vax_effect_distr_b"] = {
    datatype = "REAL",
    flag = "STEP",
    par1 = 0.0,
    par2 = 1.0,
    par3 = 0.1
}

parameters["pr_symptoms_given_flu"] = {
    datatype = "REAL",
    flag = "CONST",
    par1 = 1.0
}

parameters["pr_symptoms_given_nonflu"] = {
    datatype = "REAL",
    flag = "COPY",
    par1 = "pr_symptoms_given_flu"
}

parameters["pr_careseeking_given_vaxd"] = {
    datatype = "REAL",
    flag = "CONST",
    par1 = 1.0
}

parameters["pr_careseeking_given_unvaxd"] = {
    datatype = "REAL",
    flag = "COPY",
    par1 = "pr_careseeking_given_vaxd"
}

parameters["pr_flu_exposure"] = {
    datatype = "REAL",
    flag = "CONST",
    par1 = 0.001
}

parameters["pr_nonflu_exposure"] = {
    datatype = "REAL",
    flag = "CONST",
    par1 = 0.001
}