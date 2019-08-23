#!/usr/bin/env Rscript

library("optparse")

#Use optparse to create a list of options 
# -m multiply and stores TRUE when called
# -a add and stores TRUE when called 
# -l calculates natural logarithm of all numbers in stdin 

option_list <- list(
   make_option(c("-m", "--multiply"),
               action="store_true",
               default = FALSE,
               help="multiply stdin"), 
   make_option(c("-a", "--add"),
               action="store_true",
               default = FALSE,
               help="add stdin"), 
   make_option(c("-l", "--logprint"),
               action="store_true",
               default = FALSE,
               help="log stdin"))

###############################################################################

# Create tryInform function that throws error messages when there is a problem 

tryInform <- function(code, message) {
  handleError <- function(c) {
    c$message <- paste0(c$message, "\n", '(', message, ')')
    stop(c)
  }

  tryCatch(code, error=handleError)
}

###############################################################################

# Checks for errors in the case that the stdin file cannot be opened
f <- tryInform({f <- file('stdin'); open(f); f},
               "Could not open 'stdin' for reading")

parser <- parse_args(OptionParser(option_list=option_list))

# Throws error message if both multiply and add flag are called
if(parser$multiply && parser$add)
   stop("Cannot have multiply and add flag")

# Throws error message if neither multiply and add flag are called 
if(identical(parser$multiply, FALSE) && 
   identical(parser$add,      FALSE) &&
   identical(parser$logprint, FALSE))
   stop("Need to have either multiply or add or log flag. Try -h")

###############################################################################

# Gather all of the lines first before beginning any calculations. Note:
# this is inefficient if there are many lines of input!
lines <- c(numeric(0))
while(length(line <- readLines(f, n=1)) > 0)
  lines <- append(lines, as.numeric(line))

# Calculation for when the multiply flag is called to find the product of all 
# numbers in stdin 
if (parser$multiply)
  write(prod(lines), stdout())

# Calculation for when the add flag is called to find the sum of all numbers 
# in stdin 
if (parser$add)
  write(sum(lines), stdout())

# Print logarithm of each number
if (parser$logprint)
  write(as.character(log(lines)), stdout())

