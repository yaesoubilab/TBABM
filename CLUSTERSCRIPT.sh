#!/usr/bin/env bash
#SBATCH --partition=general
#SBATCH --job-name=TBABM_calibration
#SBATCH --output=slurm.log
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=750M

# Note: When invoking 'sbatch', you MUST specify --time=MINS and -n TASKS

# Load R, Anaconda, and TBABM. This gets all of the neccessary scripts into
# our $PATH
module load R
module load miniconda
module load TBABM/0.5.5

# Unload any existing environments and load all the R packages we will need,
# EXCEPT EasyCalibrator, which must be installed on a per-user basis, manually.
source deactivate
source activate tbabm-0.5.5 # Our custom conda environment

# There are around three tasks that aren't related to running the model but
# nevertheless take up resources: archival, likelihood calibration, and this
# process (plus deleting folders). Right now they are scrunched into one task
# (the one executing this code) but with the addition of a few 'srun' calls,
# that could change.
AUX_TASKS=1

NTASKS=$SLURM_NTASKS # Number of tasks from Slurm
NTASKS_MODEL=$((SLURM_NTASKS - AUX_TASKS)) # #tasks left for model

# Name of the archive to be created
ARCHIVE_NAME="$SLURM_JOB_NAME"'_'"$SLURM_JOB_ID"'.tar.gz'

# Make sure there's at least one task left for the model
if [[ NTASKS_MODEL -lt 1 ]]; then
  echo "Error: Fewer than 1 tasks to run model" 1>&2
  exit 1;
fi

touch monitor{1..3}.txt # files to intercept pipes

RunTBABM -c hiv_runsheets.json -i 500 -t1 -m1 -j $NTASKS_MODEL |
  CreateArchive $ARCHIVE_NAME |
    tee monitor1.txt |
  CalibrateTBABM -p 500 |
    tee monitor2.txt | 
  DeleteFolders |
  WeightLikelihoodsTBABM |
    tee monitor3.txt

echo 'Pipeline has closed. Exit status:' $?
