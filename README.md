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

### Test CPU (sequential) version:
```shell
$ module purge
$ module load intel esslurm

# build
$ make COMP=intel
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
```shell
# setup
$ module purge
$ module load PrgEnv-llvm/9.0.0-git_20190220 esslurm

# build
$ make COMP=openmp
```

### Kokkos:
```shell
# setup
$ module purge
$ module load kokkos cuda esslurm

# build
$ make COMP=kokkos
```

### Run test problem (fast, good for debugging):
```shell
$ srun ./gpp.ex test_problem
```

### Run benchmark problem (slow, this is how we will determine the hackathon winner):
```shell
$ srun ./gpp.ex benchmark_problem
```
