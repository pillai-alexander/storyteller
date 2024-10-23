.pkgs <- c("tidyverse", "tidytable", "cowplot", "geomtextpath", "here")

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
sim_dat <- fread(sim_data_path) %>%
  mutate(
    mai = ifelse((inf_symptoms == 1) & (inf_care == 1), 1, 0),
    tot_suscep = susceptibility * (1 - vax_protection)
  )

sim_duration <- 200
ts <- tidytable(
  time = numeric(),
  vax_status = numeric(),
  inf_strain = numeric(),
  mai = numeric(),
  n = numeric()
)

empty_tmp <- crossing(vax_status = c(0, 1), inf_strain = c(0, 1), mai = c(0, 1))

for (t in seq(0, sim_duration - 1, 1)) {
  tmp <- sim_dat %>%
    filter(inf_time == t) %>%
    count(vax_status, inf_strain, mai) %>%
    full_join(empty_tmp) %>%
    replace_na(list(n = 0)) %>%
    mutate(time = t)

  ts <- bind_rows(ts, tmp)
}

ts_inf_by_strain <- ts %>%
  group_by(time, inf_strain) %>%
  summarize(incid = sum(n)) %>%
  group_by(inf_strain) %>%
  mutate(cumul = cumsum(incid))

ts_flu_inf_by_vax <- ts %>%
  filter(inf_strain == 1) %>%
  group_by(time, vax_status) %>%
  summarize(incid = sum(n)) %>%
  group_by(vax_status) %>%
  mutate(cumul = cumsum(incid))

ts_mai_by_strain_vax <- ts %>%
  filter(mai == 1) %>%
  group_by(time, vax_status, inf_strain) %>%
  summarize(incid = sum(n)) %>%
  group_by(vax_status, inf_strain) %>%
  mutate(cumul = cumsum(incid))

shared_attrs <- list(
  theme_cowplot(),
  background_grid(),
  theme(
    legend.text = element_text(size = 8),
    legend.position = "inside",
    legend.position.inside = c(0.75, 0.3)
  )
)

vax_colors <- list(
  scale_color_manual(
    name = element_blank(),
    breaks = c(0, 1),
    labels = c("unvaxd", "vaxd"),
    values = c("darkorange", "dodgerblue")
  ),
  scale_fill_manual(
    name = element_blank(),
    breaks = c(0, 1),
    labels = c("unvaxd", "vaxd"),
    values = c("darkorange", "dodgerblue")
  )
)

strain_linetypes <- list(
  scale_linetype_manual(
    name = element_blank(),
    breaks = c(0, 1),
    labels = c("nonflu", "flu"),
    values = c("dotted", "solid")
  )
)

inf_by_strain <- ggplot(ts_inf_by_strain) +
  aes(x = time, y = cumul, linetype = factor(inf_strain)) +
  geom_line() +
  strain_linetypes +
  labs(x = "time", y = "cumul. infections") +
  shared_attrs

flu_inf_by_vax <- ggplot(ts_flu_inf_by_vax) +
  aes(x = time, y = cumul, color = factor(vax_status)) +
  geom_line() +
  vax_colors +
  labs(x = "time", y = "cumul. flu infections") +
  shared_attrs

mai_by_strain_vax <- ggplot(ts_mai_by_strain_vax) +
  aes(
    x = time,
    y = cumul,
    color = factor(vax_status),
    linetype = factor(inf_strain)
  ) +
  geom_line() +
  vax_colors +
  strain_linetypes +
  labs(x = "time", y = "cumul. MAIs") +
  shared_attrs +
  theme(legend.position = "none")

mean_suscep <- sim_dat %>%
  mutate(suscep = susceptibility * (1 - vax_protection)) %>%
  group_by(vax_status) %>%
  summarize(mean = mean(suscep))

true_ve <- sim_dat %>%
  filter(vax_status == 1) %>%
  summarize(true_ve = 1 - mean(vax_protection))

mean_suscep_ve_est <- mean_suscep %>%
  summarize(ve_est = 1 - (mean[vax_status == 1] / mean[vax_status == 0]))

final_tnd_ve_est <- ts_mai_by_strain_vax %>%
  filter(time == sim_duration - 1) %>%
  group_by(inf_strain) %>%
  summarize(
    vax_odds = cumul[vax_status == 1] / cumul[vax_status == 0]
  ) %>%
  ungroup() %>%
  summarize(
    tnd_ve = 1 - (vax_odds[inf_strain == 1] / vax_odds[inf_strain == 0])
  )

ve_info <- paste0(
  "True VE est.: ", signif(true_ve$true_ve, digits = 3), "\n",
  "Mean suscep. VE est.: ", signif(mean_suscep_ve_est$ve_est, digits = 3), "\n",
  "Final TND VE est.: ", signif(final_tnd_ve_est$tnd_ve, digits = 3)
)

init_suscep <- ggplot(sim_dat) +
  aes(
    x = tot_suscep,
    fill = factor(vax_status)
  ) +
  geom_histogram(
    aes(y = after_stat(ncount)),
    alpha = 0.25,
    color = "gray",
    binwidth = 0.1,
    position = "identity"
  ) +
  geom_textvline(
    data = mean_suscep,
    aes(xintercept = mean, color = factor(vax_status), label = signif(mean)),
    linetype = "dashed"
  ) +
  geom_label(
    x = max(sim_dat$tot_suscep) * 0.75,
    y = 0.5,
    label = ve_info,
    fill = "white"
  ) +
  vax_colors +
  labs(x = "susceptibility", y = "density") +
  shared_attrs +
  theme(legend.position = "none")

dash <- plot_grid(
  inf_by_strain,
  flu_inf_by_vax,
  mai_by_strain_vax,
  init_suscep
)

dir.create(fig_path)

ggsave(
  here(fig_path, "sim_dash.png"),
  dash,
  bg = "white",
  height = 1200,
  width = 2400,
  units = "px",
  dpi = 200
)
