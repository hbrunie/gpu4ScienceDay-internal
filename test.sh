#!/bin/bash

cd gpp
echo -e "\n\n\n####### Testing $PWD #######\n"

echo -e "\n\n\n####### intel #######\n"
#intel
module purge
module load intel esslurm

make clean
make COMP=intel
srun ./gpp.ex
make clean

echo -e "\n\n\n####### gcc #######\n"
#gcc
module purge
module load gcc esslurm

make COMP=gcc
srun ./gpp.ex
make clean

echo -e "\n\n\n####### cuda #######\n"
#cuda
module purge
module load cuda esslurm

make COMP=cuda
srun ./gpp.ex
make clean

echo -e "\n\n\n####### openacc #######\n"
#openacc
module purge
module load pgi cuda esslurm

make COMP=openacc
srun ./gpp.ex
make clean

echo -e "\n\n\n####### openmp #######\n"
#openmp
module purge
module load PrgEnv-llvm/9.0.0-git_20190220 esslurm

make COMP=openmp
srun ./gpp.ex
make clean

echo -e "\n\n\n####### kokkos #######\n"
#kokkos
module purge
module load kokkos cuda esslurm

make COMP=kokkos
srun ./gpp.ex
make clean

cd ../solution/cuda
echo -e "\n\n\n####### Testing $PWD #######\n"
#cuda
module purge
module load cuda esslurm

make COMP=cuda
srun ./gpp.ex
make clean

cd ../openacc
echo -e "\n\n\n####### Testing $PWD #######\n"
#openacc
module purge
module load pgi cuda esslurm

make COMP=openacc
srun ./gpp.ex
make clean

cd ../openmp
echo -e "\n\n\n####### Testing $PWD #######\n"
#openmp
module purge
module load PrgEnv-llvm/9.0.0-git_20190220 esslurm

make COMP=openmp
srun ./gpp.ex
make clean

cd ../../solution-rookie/openacc
echo -e "\n\n\n####### Testing $PWD #######\n"
#openacc
module purge
module load pgi cuda esslurm

make COMP=openacc
srun ./gpp.ex
make clean

cd ../openmp
echo -e "\n\n\n####### Testing $PWD #######\n"
#openmp
module purge
module load PrgEnv-llvm/9.0.0-git_20190220 esslurm

make COMP=openmp
srun ./gpp.ex
make clean
