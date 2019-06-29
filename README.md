# GPU for Science Day Mini-App

## Project Layout

- `^/gpp`
  - `gpp.cpp`
    - blabla...
    - blabla...
- `^/external`
  - `arrayMD/`
    - an array...
  - `ComplexClass/`
    - a complex...
- `^/solution`
  - `CUDA/`
  - `OPENACC/`
  - `OPENMP_TARGET/`

## Development Environment

```shell
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
$ module load gcc esslurm

# build
$ make
```

### CUDA:
```shell
# setup
$ module purge
$ module load cuda esslurm

# build
$ make COMP=NVCC
```

### OpenACC:
```shell
# setup
$ module purge
$ module load pgi cuda esslurm

# build
$ make COMP=pgi OPENACC=y GPU=y
```

### OpenMP:
```shell
# setup
$ module purge
$ module load PrgEnv-llvm/9.0.0-git_20190220 esslurm

# build
$ make COMP=clang OPENMP=y OPENMP_TARGET=y GPU=y
```

### Run:
```shell
$ srun ./gpp.ex
```
