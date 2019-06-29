# GPU for Science Day Mini-App

## Project Layout

- `^/GPP`
  - `gpp.cpp`
    - blabla...
    - blabla...
  - `Solutions/`
    - all you need to win the JetBot!
- `^/external`
  - `arrayMD/`
    - an array...
  - `ComplexClass/`
    - a complex...

## Development Environment

```shell
$ git clone https://github.com/to-be-fixed
$ cd gpu4science-app/GPP
```

## Cori GPU

### Get GPU node:
```shell
$ module load esslurm
$ salloc -A gpu4sci -C gpu -N 1 -t 04:00:00 -c 10 --gres=gpu:1
```

### OpenACC:
```shell
# setup
$ module purge
$ module load cuda
$ module load pgi
$ module load esslurm

# build
$ make COMP=pgi OPENACC=y GPU=y
```

### OpenMP:
```shell
# setup
$ module purge
$ module load esslurm
$ module load PrgEnv-llvm/9.0.0-git_20190220

# build
$ make COMP=clang OPENMP=y OPENMP_TARGET=y GPU=y
```

### CUDA:
```shell
# setup
$ module purge
$ module load esslurm
$ module load cuda

# build
$ make COMP=NVCC
```

### Run:
```shell
$ srun ./gpp.ex
```
