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
  here("examples", "default")
) else commandArgs(trailingOnly = TRUE)

#' setup required paths
#' assumes here() is properly at project root level
model_dir <- .args[1]
fig_path <- here(model_dir, "figs")

sim_data_path <- here(model_dir, "simvis.out")
sim_dat <- fread(sim_data_path) %>%
  mutate(
    total_flu_infs = vaxd_flu_infs + unvaxd_flu_infs,
    total_nonflu_infs = vaxd_nonflu_infs + unvaxd_nonflu_infs,
    total_flu_mais = vaxd_flu_mais + unvaxd_flu_mais,
    total_nonflu_mais = vaxd_nonflu_mais + unvaxd_nonflu_mais
  )

synthpop_path <- here(model_dir, "synthpop.out")
pop_dt <- fread(synthpop_path) %>%
  pivot_longer(!c(pid, vax_status))

means <- pop_dt %>%
  group_by(name, vax_status) %>%
  summarize(mean = signif(mean(value), 5))

print(means)

colors <- list(
  scale_color_manual(
    name = element_blank(),
    breaks = c("unvaxd", "vaxd", "ve"),
    labels = c("unvaxd", "vaxd", "ve"),
    values = c("darkorange", "dodgerblue", "darkorchid")
  ),
  scale_fill_manual(
    name = element_blank(),
    breaks = c("unvaxd", "vaxd"),
    labels = c("unvaxd", "vaxd"),
    values = c("darkorange", "dodgerblue")
  )
)

linetypes <- list(
  scale_linetype_manual(
    name = element_blank(),
    breaks = c("nonflu", "flu"),
    labels = c("nonflu", "flu"),
    values = c("dotted", "solid")
  )
)

shared_attrs <- list(
  linetypes,
  colors,
  theme_cowplot(),
  background_grid(),
  theme(
    legend.text = element_text(size = 8),
    legend.position = "inside",
    legend.position.inside = c(0.75, 0.3)
  )
)

inf_by_strain <- ggplot(sim_dat) +
  aes(x = time) +
  geom_line(aes(y = cumsum(total_flu_infs), linetype = "flu")) +
  geom_line(aes(y = cumsum(total_nonflu_infs), linetype = "nonflu")) +
  labs(x = "time", y = "cumul. infections") +
  shared_attrs

flu_inf_by_vax <- ggplot(sim_dat) +
  aes(x = time) +
  geom_line(aes(y = cumsum(vaxd_flu_infs), color = "vaxd")) +
  geom_line(aes(y = cumsum(unvaxd_flu_infs), color = "unvaxd")) +
  labs(x = "time", y = "cumul. flu infections") +
  shared_attrs

mai_by_strain_vax <- ggplot(sim_dat) +
  aes(x = time) +
  geom_line(aes(y = cumsum(vaxd_flu_mais), color = "vaxd", linetype = "flu")) +
  geom_line(aes(y = cumsum(vaxd_nonflu_mais), color = "vaxd", linetype = "nonflu")) +
  geom_line(aes(y = cumsum(unvaxd_flu_mais), color = "unvaxd", linetype = "flu")) +
  geom_line(aes(y = cumsum(unvaxd_nonflu_mais), color = "unvaxd", linetype = "nonflu")) +
  labs(x = "time", y = "cumul. MAIs") +
  shared_attrs +
  theme(legend.position = "none")

tnd_ve <- ggplot(sim_dat) +
  aes(x = time) +
  geom_line(aes(y = tnd_ve_est, color = "ve")) +
  labs(x = "time", y = "TND VE est.") +
  ylim(0, 1) +
  shared_attrs +
  theme(legend.position = "none")

pop <- ggplot(pop_dt) +
  aes(x = value, fill = factor(vax_status)) +
  geom_histogram(bins = 50, position = "identity", alpha = 0.5) +
  geom_labelvline(
    data = means,
    aes(xintercept = mean, label = mean, color = factor(vax_status)),
    alpha = 0.5
  ) +
  facet_wrap(vars(name), axes = "all", axis.labels = "all", scales = "free") +
  ylim(0, NA) +
  scale_fill_discrete(name = "Vax status") +
  scale_color_discrete(guide = "none") +
  theme_cowplot() +
  theme(legend.position = "top")

dash_left <- plot_grid(
  inf_by_strain,
  flu_inf_by_vax,
  mai_by_strain_vax,
  tnd_ve
)

dash <- plot_grid(
  dash_left,
  pop,
  nrow = 1
)

dir.create(fig_path, showWarnings = FALSE)

ggsave(
  here(fig_path, "simvis.png"),
  dash,
  bg = "white",
  height = 1200,
  width = 2400,
  units = "px",
  dpi = 200
)
