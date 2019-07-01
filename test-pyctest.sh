#!/bin/bash -e

: ${DEFAULT_ARGS:="--type=test --team=nersc"}

reset-modules()
{
    module purge
    module load esslurm python/3.6-anaconda-4.4
    source activate pyctest
    module list
}

echo -e "\n\n\n####### intel #######\n"
reset-modules
module load intel
python ./pyctest-runner.py --compiler=intel -d gpp ${DEFAULT_ARGS}

echo -e "\n\n\n####### gcc #######\n"
reset-modules
module load gcc
python ./pyctest-runner.py --compiler=gcc -d gpp ${DEFAULT_ARGS}

echo -e "\n\n\n####### cuda #######\n"
reset-modules
module load cuda
python ./pyctest-runner.py --compiler=cuda -d gpp ${DEFAULT_ARGS}

echo -e "\n\n\n####### openacc #######\n"
reset-modules
module load pgi cuda
python ./pyctest-runner.py --compiler=openacc -d gpp ${DEFAULT_ARGS}

echo -e "\n\n\n####### openmp #######\n"
reset-modules
module unload cuda
module load cuda/9.2 PrgEnv-llvm/9.0.0-git_20190220
python ./pyctest-runner.py --compiler=openmp -d gpp ${DEFAULT_ARGS}
# this does not purge for some reason
module unload PrgEnv-llvm/9.0.0-git_20190220

echo -e "\n\n\n####### kokkos #######\n"
reset-modules
module load kokkos cuda
python ./pyctest-runner.py --compiler=kokkos -d gpp ${DEFAULT_ARGS}

echo -e "\n\n\n####### Testing $PWD #######\n"
reset-modules
module load cuda
python ./pyctest-runner.py --compiler=cuda -d solution/cuda ${DEFAULT_ARGS}

echo -e "\n\n\n####### Testing $PWD #######\n"
reset-modules
module load pgi cuda
python ./pyctest-runner.py --compiler=openacc -d solution/openacc ${DEFAULT_ARGS}

echo -e "\n\n\n####### Testing $PWD #######\n"
reset-modules
module unload cuda
module load cuda/9.2 PrgEnv-llvm/9.0.0-git_20190220
python ./pyctest-runner.py --compiler=openmp -d solution/openmp ${DEFAULT_ARGS}
# this does not purge for some reason
module unload PrgEnv-llvm/9.0.0-git_20190220

echo -e "\n\n\n####### Testing $PWD #######\n"
reset-modules
module load pgi cuda
python ./pyctest-runner.py --compiler=openacc -d solution-rookie/openacc ${DEFAULT_ARGS}

echo -e "\n\n\n####### Testing $PWD #######\n"
reset-modules
module load PrgEnv-llvm/9.0.0-git_20190220
python ./pyctest-runner.py --compiler=openmp -d solution-rookie/openmp ${DEFAULT_ARGS}
