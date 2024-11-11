.pkgs <- c("tidyverse", "tidytable", "cowplot", "geomtextpath", "here",
           "paletteer")

if (interactive()) {
  stopifnot(all(sapply(.pkgs, require, character.only = TRUE)))
} else {
  suppressPackageStartupMessages(
    stopifnot(all(sapply(.pkgs, require, character.only = TRUE)))
  )
}

.args <- if (interactive()) c(
  450
) else commandArgs(trailingOnly = TRUE)

#' setup required paths
#' assumes here() is properly at project root level
model_dir <- here("exp", "basic_simulations", "discrete_suscep_recreation")
fig_path <- here(model_dir, "figs")

file_name <- paste0("synthpop_", .args[1], ".out")

pop_dt <- fread(here(model_dir, file_name)) %>%
  pivot_longer(!c(pid, vax_status))

dash <- ggplot(pop_dt) +
  aes(x = value, fill = factor(vax_status)) +
  geom_histogram(binwidth = 0.01, position = "identity", alpha = 0.5) +
  facet_wrap(vars(name)) +
  ylim(0, NA) +
  scale_fill_discrete(name = "Vax status") +
  theme_cowplot() +
  theme(legend.position = "top")

ggsave(
  here(fig_path, "popdash.png"),
  dash,
  bg = "white",
  width = 5,
  height = 5,
  units = "in",
  dpi = 150
)