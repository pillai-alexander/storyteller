.pkgs <- c("tidyverse", "tidytable", "cowplot", "here")

if (interactive()) {
  stopifnot(all(sapply(.pkgs, require, character.only = TRUE)))
} else {
  suppressPackageStartupMessages(
    stopifnot(all(sapply(.pkgs, require, character.only = TRUE)))
  )
}

#' setup required paths
#' assumes here() is properly at project root level
model_dir <- here("exp", "basic_contin_suscep_model")
fig_path <- here(model_dir, "figs")

sim_data_path <- here(model_dir, "sim.out")
sim_dat <- fread(sim_data_path)

flu_vs_nonflu <- sim_dat %>%
  filter(inf_time != -1) %>%
  group_by(inf_time, inf_strain) %>%
  summarize(incidence = n()) %>%
  group_by(inf_strain) %>%
  mutate(cumul_inf = cumsum(incidence)) %>%
  ungroup() %>%
  ggplot() +
    aes(x = inf_time, y = cumul_inf, color = factor(inf_strain)) +
    geom_step(linewidth = 2) +
    scale_color_discrete(
      name = element_blank(),
      breaks = c(0, 1),
      labels = c("nonflu", "flu")
    ) +
    theme_cowplot(20) +
    background_grid() +
    theme(
      legend.position = c(0.75, 0.1)
    ) +
    labs(x = "time", y = "cumul. infections")

vaxd_vs_unvaxd_flu <- sim_dat %>%
  filter(inf_time != -1 & inf_strain == 1) %>%
  group_by(inf_time, vax_status) %>%
  summarize(incidence = n()) %>%
  group_by(vax_status) %>%
  mutate(cumul_inf = cumsum(incidence)) %>%
  ungroup() %>%
  ggplot() +
    aes(x = inf_time, y = cumul_inf, color = factor(vax_status)) +
    geom_step(linewidth = 2) +
    scale_color_discrete(
      name = element_blank(),
      breaks = c(0, 1),
      labels = c("unvaxd", "vaxd")
    ) +
    theme_cowplot(20) +
    background_grid() +
    theme(
      legend.position = c(0.75, 0.1)
    ) +
    labs(x = "time", y = "cumul. infections")

sim_dat %>%
  ggplot() +
    aes(x = susceptibility, fill = factor(vax_status), group = vax_status) +
    geom_density()

dash <- plot_grid(
  flu_vs_nonflu,
  vaxd_vs_unvaxd_flu
)

dir.create(fig_path)

ggsave(
  here(fig_path, "sim_dash.png"),
  dash,
  bg = "white",
  height = 12,
  width = 12,
  units = "in",
  dpi = 100
)
