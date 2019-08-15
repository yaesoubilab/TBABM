#!/usr/bin/env bash

touch archive.tar 

ls-1 | xargs tar -rf archive.tar

tar -tvf archive.tar | Rscript CalibrateTBABM

tar -g archive.tar
