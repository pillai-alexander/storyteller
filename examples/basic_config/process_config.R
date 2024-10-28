.pkgs <- c("tidyverse", "tidytable", "jsonlite", "DBI", "here")

if (interactive()) {
  stopifnot(all(sapply(.pkgs, require, character.only = TRUE)))
} else {
  suppressPackageStartupMessages(
    stopifnot(all(sapply(.pkgs, require, character.only = TRUE)))
  )
}

.args <- if (interactive()) c(
  "lewnard_model.json"
) else commandArgs(trailingOnly = TRUE)

#' setup required paths
#' assumes here() is properly at project root level
model_dir <- here("examples", "basic_config")

raw_config <- read_json(
  here(model_dir, .args[1]),
  simplifyVector = TRUE
)

model_params <- as_tidytable(raw_config$model_parameters)

model_param_metadata <- model_params %>%
  select(-starts_with("par"))

variable_params <- model_params %>%
  filter(flag == "STEP") %>%
  rowwise() %>%
  mutate(vals = list(seq(from = par1, to = par2, by = par3))) %>%
  ungroup()

par_rows <- expand_grid(!!!variable_params$vals, .name_repair = "minimal")
names(par_rows) <- variable_params$fullname

const_params <- model_params %>%
  filter(flag == "CONST") %>%
  select(fullname, par1) %>%
  pivot_wider(names_from = fullname, values_from = par1) %>%
  slice(rep(seq_len(n()), each = nrow(par_rows)))

par_rows <- bind_cols(par_rows, const_params)

copy_params <- model_params %>%
  filter(flag == "COPY") %>%
  select(-c(par2, par3))

for (i in seq_len(nrow(copy_params))) {
  copied_vals <- par_rows %>%
    select(matches(copy_params$par1[i]))
  names(copied_vals) <- as.character(copy_params$fullname[i])
  par_rows <- bind_cols(par_rows, copied_vals)
}

par_rows <- par_rows %>%
  mutate(across(everything(), as.numeric)) %>%
  slice(rep(seq_len(n()), each = raw_config$n_realizations)) %>%
  group_by(everything()) %>%
  mutate(seed = seq_len(n())) %>%
  ungroup() %>%
  rowid_to_column("serial") %>%
  mutate(
    serial = as.integer(serial - 1),
    seed = as.integer(seed - 1)
  )

db_filename <- paste0(raw_config$experiment_name, ".sqlite")
db_path <- here(model_dir, db_filename)

con <- dbConnect(RSQLite::SQLite(), db_path)
dbWriteTable(con, "par", as.data.frame(par_rows), overwrite = TRUE)

met_sql <- raw_config$metrics %>%
  mutate(sql = paste0(fullname, " ", datatype))

met_table_sch <- "CREATE TABLE IF NOT EXISTS met (serial INT,"
for (i in seq_len(nrow(met_sql))) {
  cap <- ifelse(i == nrow(met_sql), ");", ",")
  met_table_sch <- paste0(met_table_sch, met_sql$sql[i], cap)
}

res <- dbSendQuery(con, met_table_sch)
dbClearResult(res)
dbDisconnect(con)