Tome = {}

-- STORYTELLER EXPERIMENT
Tome["experiment_name"] = "default example"
Tome["experiment_version"] = "0.1"
Tome["experiment_description"] = "This is the default example for the Storyteller library."

-- USER-DEFINED PATHS
-- if paths are relative, they wil be treated as relative to the tome file
Tome["database_path"] = "default.sqlite"
Tome["output_dir_path"] = "out"

-- GLOBAL PARAMETERS
Tome["n_realizations"] = 10
Tome["par_value_tolerance"] = 1e-10

-- CONFIGURATION TABLE OF CONTENTS
Tome["parameters"] = "config/parameters.lua"
Tome["metrics"] = "config/metrics.lua"