## Installing within SLURM environment

In order to run TBABM on the cluster, you must compile the binaries in a similar environment to the one they will be executed on. First, acquire an interactive allocation with several cores, to speed compile time.

```bash
srun --pty -p interactive --cpus-per-task=8 --time=60 bash
```

You should now be inside an interactive job allocation which will allow you to build binaries in the correct environment

### Installing dependencies

TBABM references a few in-house libraries, **SimulationLib** and **StatisticalDistributionsLib**. They must be installed in a directory that TBABM's build system, **CMake**, can see when you are compiling the TBABM binary.

First, we will create a temporary directory to compile all the software, and a `software/` directory to house the compiled binaries and headers. We'll also make a special variable, `$PREFIX`, to remind us of where `software/` is.

```bash
mkdir ~/software ~/tmp && cd ~/tmp
PREFIX="~/software"
export PREFIX
```

Next, we load a toolchain, and load CMake, our build system:

```bash
module load CMake/3.9.1 foss/2018b || echo "Loading one or more modules failed!"
```

Next, clone all the repositories:

```bash
echo https://github.com/yaesoubilab/{TBABM,SimulationLib,StatisticalDistributionsLib}.git |
  xargs -n1 git clone
```

Compile and install them:

```bash
cd StatisticalDistributionsLib
cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -DCMAKE_BUILD_TYPE=RelWithDebInfo .
make -j8 install

cd ../SimulationLib/SimulationLib
cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -DCMAKE_BUILD_TYPE=RelWithDebInfo .
make -j8 install
```

Check the contents of `~/software/` to see if the installation succeeded. If it did, the directory structure should look as follows:

```
software/
├── include
│   ├── SimulationLib-0.2
│   └── StatisticalDistributionsLib-0.2
└── lib
    ├── SimulationLib-0.2
    │   └── RelWithDebInfo
    └── StatisticalDistributionsLib-0.2
        └── RelWithDebInfo
```

You can go ahead and delete `~/tmp` now.

```bash
rm -rf ~/tmp
```

### Installing TBABM

```bash
VERSION=0.6.6

mkdir ~/pkg ~/modulefiles/TBABM
module use ~/modulefiles

cd ~/pkg
git clone --branch=$VERSION https://github.com/yaesoubilab/TBABM.git $VERSION/

cd $VERSION
cmake -DCMAKE_PREFIX_PATH=$PREFIX -DCMAKE_BUILD_TYPE=RelWithDebInfo .
make -j8

cd ~/modulefiles/TBABM
nano $VERSION.lua
```

Paste the following into the editor:

```lua
local home      = os.getenv("HOME")
local version   = myModuleVersion()
local pkgName   = myModuleName()
local pkg       = pathJoin(home, "pkg",pkgName,version,"bin")
prepend_path("PATH", pkg)
```

## Running TBABM on a SLURM cluster

### Setting up an execution environment

```bash
mkdir ~/TBABM_runs/run1 && cd ~/TBABM_runs/run1
cp -R ~/pkg/TBABM/bin/CLUSTERSCRIPT.sh ~/pkg/TBABM/params/{HIV,Demographic}/ .
```

Add yer runsheet

### Performing the run

```bash
sbatch -n200 --time=500 CLUSTERSCRIPT.sh &&
	watch -n1 squeue -u$USER

less +F slurm.log # This gives a stream of information that will indicate errors
				  # in setup of the various processes involved

less +F parallel.log # This is useful for monitoring runtimes

```
