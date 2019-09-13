#!/usr/bin/env bash
#SBATCH --partition=scavenge
#SBATCH --job-name=TBABM_calibration
#SBATCH --output=slurm.log
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=2000M

# Note: When invoking 'sbatch', you MUST specify --time=MINS and -n TASKS

echo "Running on $(hostname)"

# Load R, Anaconda, and TBABM. This gets all of the neccessary scripts into
# our $PATH
module load foss/2018b parallel
module load R miniconda

module use ~/modulefiles
module load TBABM/0.5.5-beta1

# Unload any existing environments and load all the R packages we will need,
# EXCEPT EasyCalibrator, which must be installed on a per-user basis, manually,
# before execution of this script.
source deactivate
source activate tbabm-0.5.5 # Our custom conda environment

# There are around three tasks that aren't related to running the model but
# nevertheless take up resources: archival, likelihood calibration, and this
# process (plus deleting folders). Right now they are scrunched into one task
# (the one executing this code) but with the addition of a few 'srun' calls,
# that could change.
AUX_TASKS=1

NTASKS=$SLURM_NTASKS # Number of tasks from Slurm
NTASKS_MODEL=$((SLURM_NTASKS - AUX_TASKS)) # number of tasks left for model

# Name of the archive to be created
ARCHIVE_NAME="$SLURM_JOB_NAME"'_'"$SLURM_JOB_ID"'.tar'

mkdir -p tmpdir || {(>&2 echo 'Creation of tmpdir/ failed'); exit 1;}
TMPDIR="$(pwd)/tmpdir"
export TMPDIR

touch monitor{1..3}.txt # files to intercept pipes

# Make sure there's at least one task left for the model
if [[ NTASKS_MODEL -lt 1 ]]; then
  echo "Error: Fewer than 1 tasks to run model" 1>&2
  exit 1;
fi

time RunTBABM -c runsheets.json -i 30000 -t1 -m1 -j $NTASKS_MODEL |
#   CreateArchive "$ARCHIVE_NAME" |
    tee monitor1.txt |
  CalibrateTBABM |
    tee monitor2.txt | 
  DeleteFolders |
  WeightLikelihoodsTBABM |
    tee monitor3.txt

echo 'Pipeline has closed. Exit status:' $? 1>&2

rm -rf "$TMPDIR"

