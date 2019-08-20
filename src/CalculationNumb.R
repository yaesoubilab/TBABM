library("optparse")

option_list <- list(
   make_option(c("-m", "--multiply"), action="store_true", default = FALSE, help="multiply stdin"), 
   make_option(c("-a", "--add"), action="store_true", default = FALSE, help="add stdin"), 
   make_option(c("-l", "--logprint"), action="store_true", default = FALSE, help="log stdin"))

parser <- parse_args(OptionParser(option_list=option_list))

f <- file('stdin')
open(f)

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

 
