#!/usr/bin/env Rscript

# Make tidyverse quiet!
suppressPackageStartupMessages(library(tidyverse, quietly=TRUE, 
                   verbose=FALSE,
                   warn.conflicts=FALSE))
library(tools, quietly=TRUE, warn.conflicts=FALSE)
library(jsonlite, quietly=TRUE, warn.conflicts=FALSE)
library(optparse, quietly=TRUE, warn.conflicts=FALSE)
library(assertthat, quietly=TRUE, warn.conflicts=FALSE)

#####################################################
## Error handling
#####################################################
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

#####################################################
## Specs for various input and output formats
#####################################################

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

substitution_spec <- cols(
  run.id   = col_character(),
  .default = col_double()
)

# Spec for the rangefile
rangefile_spec <- cols(
  name  = col_character(),
  lower = col_double(),
  upper = col_double(),
  step  = col_double()
)

#####################################################
## Sampling functions
#####################################################
GenSamples <- function(input, n) {
  tp <- transpose(input) %>% setNames(input$name)
  map(tp, ~runif(n, .$min, .$max))
}

GenSamples_df <- function(GenSamples_output) {
  tbl <- as_tibble(GenSamples_output)

  mutate(tbl, run.id=1:nrow(tbl)) %>%
    select(run.id, everything())
}

#####################################################
## Combinatorial code – for RangeFiles
#####################################################

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

#####################################################
## Code for generating runsheets
#####################################################

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
  
  GenRunSheets_pf_impl(proto, priors, n)
}

GenRunSheets_pf_impl <- function(proto, prior, n) {

  sampled <- GenSamples(prior, n)

  list(dfs = map(pmap(sampled, list), GenRunSheet, proto),
       vs  = sampled)
}

# proto_fname, runsheets_fname, priorfile_fname -> 
# 'proto_fname': The filename of a prototype runsheet
#
# 'substitutions_fname': The filename specifying the substitutions that have been
#                    done to that runsheet. Aka, the .csv output of a previous
#                    gen_params.R call without the '-s' flag.
#
# 'priorfile_fname': The priorfile used to produce variations on each runsheet
#                    specified by the combo of 'proto_fname' and 'runsheets_fname'
GenRunSheets_subst <- function(proto_fname, substitutions_fname, priorfile_fname, n=1) {

  proto             <- read_csv(proto_fname,         col_types=runsheet_spec)
  substitutions     <- read_csv(substitutions_fname, col_types=substitution_spec)
  priors            <- read_csv(priorfile_fname,     col_types=priorfile_spec)

  message("Performing substitutions on prototype_file")
  # Take the prototype runsheet, perform all the substitutions that were asked
  # for by the substitutions file
  substituted_runsheets <- map(transpose(substitutions), GenRunSheet, proto)
  
  message("Performing uniform sampling from new prior on substituted runsheets")
  # For each one of these runsheets that have bene substituted, take 'n'
  # samples from the prior and substitute each set of samples into a runsheet.
  # Do this for every runsheet contained in 'substituted'
  sampled <- map(substituted_runsheets, GenRunSheets_pf_impl, priors, n)

  message("Accumulating all runsheets")
  # The last call returns a kind of ugly structure. We want the output of the
  # function to be identical to GenRunSheets, for export reasons. So, here is 
  # a bunch of black magic that does that.
  final <- reduce(sampled,
                  ~list(dfs=append(.x$dfs,  .y$dfs),
                        vs=bind_rows(.x$vs, .y$vs)),
                  .init=list(dfs=list(), vs=tibble()))

  # This^ approach depended on converting a list to an dataframe so that
  # bind_rows could be used. Here, we convert the dataframe back to a list
  # to maintain parity with the return value of GenRunSheets.
  final$vs <- as.list(final$vs)

  final
}

#####################################################
## Argument processing
#####################################################
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
              help="(-p only) Number of samples from joint prior"),
  make_option(c("-s", "--substitute"),
              action="store_true",
              default=FALSE,
              help="Treat PROTOTYPE_FILE as a csv and create variants on each row using PRIORFILE Implies -p."),
  make_option(c("-o", "--output-prefix"),
              action="store",
              default="runsheets",
              help="Prefix of .csv,.json files to be output. Default is 'runsheets'"))

usage <- "%prog [options] PROTOTYPE_FILE RANGEFILE/PRIORFILE\n
%prog -s [options] PROTOTYPE_FILE RUNSHEETS_FILE PRIORFILE"

description <- "Creates runsheets from either rangefiles or priorfiles"

#####################################################
## Main function
#####################################################
main <- function(args) {
  parser <- OptionParser(option_list=option_list,
                         usage=usage,
                         prog="./gen_runsheets.R",
                         description=description)

  opts <- parse_args(parser, args=args, positional_arguments=TRUE)$options
  args <- parse_args(parser, args=args, positional_arguments=TRUE)$args

  if (opts$range & opts$prior)
    die("Must specify -p flag when using -s")
  if (opts$range & opts$substitute)
    die("Can't specify -r and -s at the same time, for now")
  if (opts$substitute && length(args) < 3)
    die("-s flag requires three arguments. See -h")
  if (opts$prior & opts$range)
    die("Can't specify prior and range at the same time")
  if (! (opts$prior || opts$range))
    die("Must specifiy either -p or -rn")
  if (opts$prior && identical(opts$`num-samples`, 0))
    die("When using -p, must specify number of samples")
  if (!opts$substitute && length(args) != 2)
    die("Must specify PROTOTYPE_FILE RANGEFILE/PRIORFILE")

  proto_fname <- args[1]
  file_fname  <- args[2]
  
  if (opts$substitute) {
    message("Generating runsheets")
    runsheets <- GenRunSheets_subst(proto_fname, # the proto runsheet
                                    file_fname,  # the runsheets we're substituting into
                                    args[3],     # the prior file we're using to drive the substition
                                    opts$`num-samples`)
    vs        <- runsheets$vs
    runsheets <- runsheets$dfs
  } else if (opts$range) {
    runsheets <- GenRunSheets_rf(proto_fname, file_fname)
  } else if (opts$prior) {
    runsheets <- GenRunSheets_pf(proto_fname,
                                 file_fname,
                                 opts$`num-samples`)
    vs        <- runsheets$vs
    runsheets <- runsheets$dfs
  }

  csvname  <- paste0(opts$`output-prefix`, '.csv')
  jsonname <- paste0(opts$`output-prefix`, '.json')

  writer <- function(runsheet) {
    json <- toJSON(runsheet)
    write(json,
          file=jsonname,
          append=TRUE)
  }

  message("Generating .json from runsheets and writing to disk")
  tryInform(walk(runsheets, writer), "Writing of collated runsheet failed")
  message("Finished writing json")

  if (opts$range) {
    message("Writing .csv to disk")
    tryInform(write_csv(GenCombinations_df(file_fname), csvname),
              "Writing of runsheet table failed")
    message("Finished writing .csv")
  }
  else if (opts$prior || opts$substitute) {
    message("Writing .csv to disk")
    tryInform(write_csv(GenSamples_df(vs),
                        csvname),
              "Writing of runsheet table failed")
    message("Finished writing .csv")
  }
}

main(commandArgs(trailingOnly=TRUE))
