# GPU for Science Day Mini-App

## Project Layout

- `^/gpp`
  - `gpp.cpp`
    - main function and the kernel to focus on
  - `Makefile`
- `^/external`
  - `commonDefines.h`
  - `arrayMD/`
    - host-device data array
  - `ComplexClass/`
    - custom complex number class
- `^/solution-rookie`
  - `openacc/`
  - `openmp/`
- `^/solution`
  - `cuda/`
  - `openacc/`
  - `openmp/`

## Development Environment

```shell
$ cd $SCRATCH
$ git clone https://github.com/to-be-fixed
$ cd gpu4science-app/gpp
```

## Cori GPU

### Get GPU node:
```shell
$ module load esslurm
$ salloc -A gpu4sci -C gpu -N 1 -t 04:00:00 -c 10 --gres=gpu:1
```

### Build CPU (sequential) version:
```shell
$ module purge
$ module load intel esslurm

# build
$ make COMP=intel
$ ./gpp.ex
```

### CUDA:
```shell
# setup
$ module purge
$ module load cuda esslurm

# build
$ make COMP=cuda
```

### OpenACC:
```shell
# setup
$ module purge
$ module load pgi cuda esslurm

# build
$ make COMP=openacc
```

### OpenMP:
#### Info: For the Cori GPU Skylake CPU, set OMP_NUM_THREADS=10. (This will not affect the GPU.)
```shell
# setup
$ module purge
$ module load PrgEnv-llvm/9.0.0-git_20190220 esslurm

# build
$ make COMP=openmp
$ export OMP_NUM_THREADS=10
```

### Kokkos:
```shell
# setup
$ module purge
$ module load kokkos cuda esslurm

# build
$ make COMP=kokkos
```

### Run debug_application (fast, good for debugging):
#### Hint: Do NOT optimize the test problem, it runs so quickly it is not representative
```shell
$ srun ./gpp.ex debug_application
```

### Run real_application (slow, this is how we will determine the hackathon winner):
```shell
$ srun ./gpp.ex real_application
```

## Competition Submission

1. Decide on a team name, if you have not done so already
   - In the steps below, replace `TEAM_NAME` with this name
2. Go to the top-level directory
3. Create a branch for your team
   - `git checkout -b TEAM_NAME`
4. Check to make sure your directory is clean
   - `git status`
   - remove any build files or outputs that show up
   - Do not add/commit any files other than code
5. Stage the files you want to commit
   - `git add gpp/gpp.cpp` and any other relevant **text** files, e.g. `gpp/gpp.cu`
6. Commit the code
   - `git commit -m "Official submission of TEAM_NAME"`
7. Push the code upstream
   - `git push`
8. Execute the PyCTest script
   - `python ./pyctest-runner.py --team=TEAM_NAME --compiler=COMPILER`
     - `TEAM_NAME` should be your team name... Please do not copy/paste and submit as `TEAM_NAME`
     - `COMPILER` should be one of `openacc`, `openmp`, `cuda`, or `kokkos`
   - This script will:
   - Build the code in the `gpp` folder
   - Execute the benchmark test
   - Submit the build and test logs to [cdash.nersc.gov](https://cdash.nersc.gov/index.php?project=gpu4science-app)
