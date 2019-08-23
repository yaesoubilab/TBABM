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
if(parser$multiply & parser$add){
   write("Cannot have multiply and add flag", stderr())
   stop()
}

# Throws error message if neither multiply and add flag are called 
if(parser$multiply==FALSE & parser$add==FALSE){
   write("Need to have either multiply or add flag", stderr())
   stop()
}

###############################################################################

# Calculation for when the multiply flag is called to find the product of all 
# numbers in stdin 
if(parser$multiply){
   mult <- 1
   while(length(line <- readLines(f, n = 1)) > 0){
      l <- as.numeric(line)
      mult <- mult * l  
   }
   print(mult)
}

# Calculation for when the add flag is called to find the sum of all numbers 
# in stdin 
if(parser$add){
   sum <- 0
   while(length(line <- readLines(f, n = 1)) > 0){
      k <- as.numeric(line)
      sum <- sum + k
   }
   print(sum)
}

# Calculationf or when the logprint flag is called to find the natural log of 
# all numbers in stdin
if(parser$logprint){
   while(length(line <- readLines(f, n = 1)) > 0){
      j <- as.numeric(line)
      numb <- log(j)
      print(numb)
   }
}

 
