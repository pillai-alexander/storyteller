Tome = {}

-- GENERIC STORYTELLER PARAMETERS
Tome["experiment_name"] = "default"
Tome["database_path"] = "baseline_performance.sqlite"
Tome["n_realizations"] = 10
Tome["par_value_tolerance"] = 1e-10
Tome["output_dir_path"] = "out"

-- CONFIGURATION TABLE OF CONTENTS
Tome["parameters"] = "config/parameters.lua"
Tome["metrics"] = "config/metrics.lua"