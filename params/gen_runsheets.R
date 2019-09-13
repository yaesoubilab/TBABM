#!/usr/bin/env Rscript

# Make tidyverse quiet!
suppressPackageStartupMessages(library(tidyverse, quietly=TRUE, 
                   verbose=FALSE,
                   warn.conflicts=FALSE))
library(tools, quietly=TRUE)
library(jsonlite, quietly=TRUE, warn.conflicts=FALSE)
library(optparse, quietly=TRUE)

tryInform <- function(code, message) {
  handleError <- function(c) {
    c$message <- paste0(c$message, "\n", '(', message, ')')
    stop(c)
  }

  tryCatch(code, error=handleError)
}

die <- function(message) {
  write(paste0("Error: ", message), stderr())
  quit(status=1)
}

option_list <- list(
  make_option(c("-p", "--prior"),
              action="store_true",
              default=FALSE,
              help="Create a collated runsheet from uniform prior distributions"),
  make_option(c("-r", "--range"),
              action="store_true",
              default=FALSE,
              help="Create a collated runsheet from the cartesian product of sequences"),
  make_option(c("-n", "--num-samples"),
              action="store",
              default=0,
              help="(-p only) Number of samples from joint prior"))

usage <- "%prog [options] PROTOTYPE_FILE RANGEFILE/PRIORFILE"
description <- "Creates runsheets from either rangefiles or priorfiles"

# Spec for the RunSheet
runsheet_spec <- cols(
  description               = col_character(),
  `short-name`              = col_character(),
  type                      = col_character(),
  distribution              = col_character(),
  `parameter-description`   = col_character(),
  `parameter-1`             = col_double(),
  `parameter-2`             = col_double(),
  `parameter-3`             = col_double(),
  `parameter-4`             = col_double(),
  `included-in-calibration` = col_logical()
)

# Spec for the priorfile
priorfile_spec <- cols(
  name  = col_character(),
  min   = col_double(),
  max   = col_double()
)

# Spec for the rangefile
rangefile_spec <- cols(
  name  = col_character(),
  lower = col_double(),
  upper = col_double(),
  step  = col_double()
)

GenSamples <- function(input, n) {
  tp <- transpose(input) %>% setNames(input$name)
  map(tp, ~runif(n, .$min, .$max))
}

GenSamples_df <- function(GenSamples_output, n) {
  mutate(as_tibble(GenSamples_output), run.id=1:n) %>%
    select(run.id, everything())
}

# Cross all of the different parameter combinations together. The result of
# this call is an unkeyed list of lists, where the innermost lists contain k-v
# pairs. Each innermost list represents a set of parameters that will be used
# to modify the prototype RunSheet. Thus, the keys of the innermost lists
# correspond to the 'short-name' of a parameter, and the values, for right
# now, are what 'parameter-1' should be set to.
GenCombinations <- function(substitutions) {
  pmap(substitutions, 
       function(name, lower, upper, step) seq(lower, upper, step)) %>%
    setNames(substitutions$name) %>%
    cross()
}

# fname -> tibble containing a row for each run where the columns are
# the various parameters
GenCombinations_df <- function(rangefile_fname) {
  substitutions <- read_csv(rangefile_fname, col_types=rangefile_spec)

  pmap(substitutions,
       function(name, lower, upper, step) seq(lower, upper, step)) %>%
       setNames(substitutions$name) %>%
       cross_df() %>%
       mutate(run.id = seq(n())) %>%
       select(run.id, everything())
}

# Given a list of substitutions, outputs a new parameter sheet, as a tibble, which
# includes these substitutions
# 'substitutions" is a keyed list, containing 'short-name':'parameter-1' pairs.
# 
# Right now, substitution can only be done on 'parameter-1', though this may change
GenRunSheet <- function(substitutions, new_runsheet) {
  slotter <- function(runsheet, name) {
    row <- which(runsheet$`short-name` == name)
    column <- 'parameter-1'
    
    runsheet[row, column] <- substitutions[[name]]
    runsheet
  }
  
  reduce(names(substitutions), slotter, .init=new_runsheet)
}

GenRunSheets_rf <- function(proto_fname, rangefile_fname) {
  
  # Read the prototype file and the substitutions file in and convert them to
  # tibbles
  proto         <- read_csv(proto_fname,     col_types=runsheet_spec)
  substitutions <- read_csv(rangefile_fname, col_types=rangefile_spec)
  
  # Cross all of the different parameter combinations together. The result of
  # this call is an unkeyed list of lists, where the innermost lists contain k-v
  # pairs. Each innermost list represents a set of parameters that will be used
  # to modify the prototype RunSheet. Thus, the keys of the innermost lists
  # correspond to the 'short-name' of a parameter, and the values, for right
  # now, are what 'parameter-1' should be set to.
  crossed <- GenCombinations(substitutions)
  
  map(crossed, GenRunSheet, proto)
}

GenRunSheets_pf <- function(proto_fname, priorfile_fname, n) {
  
  # Read the prototype file and the substitutions file in and convert them to
  # tibbles
  proto  <- read_csv(proto_fname,     col_types=runsheet_spec)
  priors <- read_csv(priorfile_fname, col_types=priorfile_spec)
  
  sampled <- GenSamples(priors, n)

  list(dfs = map(pmap(sampled, list), GenRunSheet, proto),
       vs  = sampled)
}

main <- function(args) {
  parser <- OptionParser(option_list=option_list,
                         usage=usage,
                         prog="./gen_runsheets.R",
                         description=description)

  opts <- parse_args(parser, args=args, positional_arguments=TRUE)$options
  args <- parse_args(parser, args=args, positional_arguments=TRUE)$args

  if (opts$prior & opts$range)
    die("Can't specify prior and range at the same time")
  if (! (opts$prior || opts$range))
    die("Must specifiy either -p or -rn")
  if (opts$prior && !opts$`num-samples`)
    die("When using -p, must specificy number of samples")
  if (length(args) != 2)
    die("Must specify PROTOTYPE_FILE RANGEFILE/PRIORFILE")

  proto_fname <- args[1]
  file_fname  <- args[2]
  
  if (opts$range)
    runsheets <- GenRunSheets_rf(proto_fname, file_fname)
  else if (opts$prior) {
    runsheets <- GenRunSheets_pf(proto_fname,
                                 file_fname,
                                 opts$`num-samples`)
    vs        <- runsheets$vs
    runsheets <- runsheets$dfs
  }

  writer <- function(runsheet) {
    json <- toJSON(runsheet)
    write(json,
          file="runsheets.json",
          append=TRUE)
  }

  tryInform(walk(runsheets, writer), "Writing of collated runsheet failed")

  if (opts$range)
    tryInform(write_csv(GenCombinations_df(file_fname), 'runsheets.csv'),
              "Writing of runsheet table failed")
  else
    tryInform(write_csv(GenSamples_df(vs, opts$`num-samples`),
                        'runsheets.csv'),
              "Writing of runsheet table failed")
}

main(commandArgs(trailingOnly=TRUE))
