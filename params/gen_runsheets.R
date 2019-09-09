#!/usr/bin/env Rscript

# Make tidyverse quiet!
suppressPackageStartupMessages(library(tidyverse, quietly=TRUE, 
                   verbose=FALSE,
                   warn.conflicts=FALSE))
library(tools, quietly=TRUE)
library(jsonlite, quietly=TRUE, warn.conflicts=FALSE)
library(optparse, quietly=TRUE)


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
  lower = col_double(),
  upper = col_double()
)

# Spec for the rangefile
rangefile_spec <- cols(
  name  = col_character(),
  lower = col_double(),
  upper = col_double(),
  step  = col_double()
)

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
  substitutions <- read_csv(rangefile_fname, col_types=rangefile_spec) %>%
    as.tibble()

  pmap(substitutions,
       function(name, lower, upper, step) seq(lower, upper, step)) %>%
       setNames(substitutions$name) %>%
       cross_df() %>%
       mutate(run.id = seq(n())) %>%
       select(run.id, everything())
}

GenRunSheets <- function(proto_fname, rangefile_fname) {
  
  # Read the prototype file and the substitutions file in and convert them to
  # tibbles
  proto <- read_csv(proto_fname, col_types=runsheet_spec) %>% as_tibble()

  substitutions <- read_csv(rangefile_fname, col_types=rangefile_spec) %>% 
    as_tibble()
  
  # Cross all of the different parameter combinations together. The result of
  # this call is an unkeyed list of lists, where the innermost lists contain k-v
  # pairs. Each innermost list represents a set of parameters that will be used
  # to modify the prototype RunSheet. Thus, the keys of the innermost lists
  # correspond to the 'short-name' of a parameter, and the values, for right
  # now, are what 'parameter-1' should be set to.
  crossed <- GenCombinations(substitutions)
  
  # Given a list of substitutions, outputs a new parameter sheet, as a tibble, which
  # includes these substitutions
  # 'substitutions" is a keyed list, containing 'short-name':'parameter-1' pairs.
  # 
  # Right now, substitution can only be done on 'parameter-1', though this may change
  GenRunSheet <- function(substitutions, new_runsheet) {
    for (name in names(substitutions)) {
      row <- which(new_runsheet$`short-name` == name)
      column <- 'parameter-1'
      
      new_runsheet[row, column] <- substitutions[[name]]
    }
    
    new_runsheet
  }
  
  crossed %>% map(~GenRunSheet(., proto))
}

WriteRunSheets <- function(runsheets, prefix="RunSheet_") {
  
  # Determine the number of digits needed to represent the ID of the
  # last RunSheet
  num_sheets <- length(runsheets)
  num_digits <- floor(log10(num_sheets)) + 1
  
  WriteRunSheet <- function(runsheet, sheet_id) {
    print(runsheet)
    
    # Format the RunSheet ID: add an appropriate amount of leading 0's
    number <- formatC(sheet_id, width=num_digits, format="d", flag="0")
    
    # Generate paths to write .csv and .json file to
    path_csv <- paste0(prefix, number, ".csv")
    path_json <- paste0(prefix, number, ".json")
    
    # Write the .csv and .json files to disk. It's important to make sure NA is
    # represented as the empty string, so that an empty cell is represented as
    # ",,".
    write_json(runsheet, path_json, na='null')
  }
  
  map2(runsheets, seq(num_sheets), WriteRunSheet)
}

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

main <- function(args) {

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
                help="(-p only) Number of samples from joint prior")
  )
  
  usage <- "%prog [options] PROTOTYPE_FILE RANGEFILE/PRIORFILE"
  description <- "Creates runsheets from either rangefiles or priorfiles"

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

  print(opts)
  proto_fname     <- args[1]
  rangefile_fname <- args[2]
  
  GenPrefix <- function(rangefile_fname) return("")
  
  runsheets <- GenRunSheets(proto_fname, rangefile_fname)
  
  collate <- TRUE
  if (collate) {
    map(runsheets, ~toJSON(., na='null')) %>% 
      paste(collapse="\n") %>% 
      write(file="runsheets.json")
    
    # iwalk(runsheets, ~write_csv(.x, "runsheets.csv", append=(.y != 1)))
  } else {
    WriteRunSheets(runsheets, prefix="")
  }
  
  write_csv(GenCombinations_df(rangefile_fname), 'runsheets.csv')
}

main(commandArgs(trailingOnly=TRUE))

