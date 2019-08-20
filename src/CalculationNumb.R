library("optparse")

option_list <- list(
   make_option(c("-m", "--multiply"), action="store_true", default = FALSE, help="multiply stdin"), 
   make_option(c("-a", "--add"), action="store_true", default = FALSE, help="add stdin"), 
   make_option(c("-l", "--logprint"), action="store_true", default = FALSE, help="log stdin"))

parser <- parse_args(OptionParser(option_list=option_list))

#####################################################################################################     
tryInform <- function(code, message) {
  handleError <- function(c) {
    c$message <- paste0(c$message, "\n", '(', message, ')')
    stop(c)
  }

  tryCatch(code, error=handleError)
}


#####################################################################################################     

f <- tryInform({f <- file('stdin'); open(f); f},
              "Could not open 'stdin' for reading")

if(parser$multiply & parser$add){
   write("Cannot have multiply and add flag", stderr())
   stop()
}

if(parser$multiply==FALSE & parser$add==FALSE){
   write("Need to have either multiply or add flag", stderr())
   stop()
}

#####################################################################################################     

if(parser$multiply){
   mult <- 1
   while(length(line <- readLines(f, n = 1)) > 0){
      l <- as.numeric(line)
      mult <- mult * l  
   }
   print(mult)
}

if(parser$add){
   sum <- 0
   while(length(line <- readLines(f, n = 1)) > 0){
      k <- as.numeric(line)
      sum <- sum + k
   }
   print(sum)
}

if(parser$logprint){
   while(length(line <- readLines(f, n = 1)) > 0){
      j <- as.numeric(line)
      numb <- log(j)
      print(numb)
   }
}

 
