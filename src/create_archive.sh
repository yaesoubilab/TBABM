#!/usr/bin/env bash

#create archive.tar folder
touch archive.tar 

# pipe in the list of all folders and pipe them into tar with xargs
ls-1 | xargs tar -rf archive.tar

#pipe in the stdout from archives to CalibrateTBABM
tar -tvf archive.tar | Rscript CalibrateTBABM

# gunzip the archive
tar -g archive.tar
