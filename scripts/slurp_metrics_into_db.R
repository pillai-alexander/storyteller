.pkgs <- c("tidytable", "DBI", "pbapply")

if (interactive()) {
  stopifnot(all(sapply(.pkgs, require, character.only = TRUE)))
} else {
  suppressPackageStartupMessages(
    stopifnot(all(sapply(.pkgs, require, character.only = TRUE)))
  )
}

.args <- if (interactive()) c(
  "/home/anpillai/documents/work/personal/storyteller/examples/default/default.sqlite",
  "/home/anpillai/documents/work/personal/storyteller/examples/default/out"
) else commandArgs(trailingOnly = TRUE)

files <- list.files(.args[2], full.names = TRUE)

csv_to_db <- function(file_path, db) {
  dbWriteTable(db, "met", as.data.frame(fread(file_path)), append = TRUE)
}

pboptions(type = "timer", char = ":")

con <- dbConnect(RSQLite::SQLite(), .args[1])
dbBegin(con)
invisible(pbapply::pbsapply(files, csv_to_db, db = con))
dbCommit(con)
dbDisconnect(con)
