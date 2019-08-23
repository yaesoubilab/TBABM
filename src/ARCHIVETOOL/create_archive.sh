#!/usr/bin/env bash

# -z gunzip the archive that is created 
# -r appends to a file called archive.tar.gz
# -f file name -> name of the archive file we are creating 

xargs -n1 tar -vrf $1 
