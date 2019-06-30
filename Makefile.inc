NVCC:=nvcc
OPENMP:=n
OPENMP_TARGET:=n
OPENACC:=n
KOKKOS:=n
GPU:=n
GPU_OPENACC_UM:=n
GPU_CUDA_UM:=n
BENCHMARK:=n
CUDA_PATH?=$(CUDA_ROOT)
USE_OPENMP_SCHEDULE:=n
USE_INLINE_MATH:=n

ifeq ($(findstring lang,$(COMP)),lang)
     CC=clang
     CXX=clang++
     FC=flang
     F77=flang
     COMP_CHOICE=clang
     LINKER_PREFIX=-Wl,
else ifeq ($(findstring xl,$(COMP)),xl)
     CC=xlc_r
     CXX=xlC_r
     FC=xlf90_r
     F77=xlf_r
     COMP_CHOICE=xl
     LINKER_PREFIX=-Wl,
else ifeq ($(findstring pg,$(COMP)),pg)
     CC=pgcc
     CXX=pgc++
     FC=pgf90
     F77=pgf77
     COMP_CHOICE=pgi
     LINKER_PREFIX=-Wl,
else ifeq ($(findstring ic,$(COMP)),ic)
     CC=icc
     CXX=icpc
     FC=ifort
     F77=ifort
     COMP_CHOICE=intel
     LINKER_PREFIX=-Wl,
else ifeq ($(findstring gc,$(COMP)),gc)
     CC=gcc
     CXX=g++
     FC=gfortran
     F77=gfortran
     COMP_CHOICE=gcc
     LINKER_PREFIX=-Wl,
else ifeq ($(findstring gf,$(COMP)),gf)
     CC=gcc
     CXX=g++
     FC=gfortran
     F77=gfortran
     COMP_CHOICE=gcc
     LINKER_PREFIX=-Wl,
else ifeq ($(findstring g+,$(COMP)),g+)
     CC=gcc
     CXX=g++
     FC=gfortran
     F77=gfortran
     COMP_CHOICE=gcc
     LINKER_PREFIX=-Wl,
else ifeq ($(findstring NVCC,$(COMP)),NVCC)
     CC=nvcc
     CXX=nvcc
     COMP_CHOICE=nvcc
     LINKER_PREFIX=-Xlinker=
else ifeq ($(findstring kokkos,$(COMP)),kokkos)
     CC=nvcc_wrapper
     CXX=nvcc_wrapper
     COMP_CHOICE=kokkos
     LINKER_PREFIX=-Xlinker=
else
     $(warning Warning COMP is not set)
endif

# We can use OpenMP target code on the host and the GPU
ifeq ($(OPENMP_TARGET),y)
  override CPPFLAGS += -DOPENMP_TARGET
endif

# We can use OpenMP target code on the host and the GPU
ifeq ($(BENCHMARK),y)
  override CPPFLAGS += -DBENCHMARK
endif

# Allow the user to override CPPFLAGS only, e.g.
# make CC=gcc CPPFLAGS=-DSTREAM_ARRAY_SIZE=100000
ifeq ($(GPU),y)
  ifeq ($(OPENMP_TARGET),y)
    # We can only use OpenMP target code for the GPU
    override CPPFLAGS += -DGPU
  endif
  ifeq ($(OPENACC),y)
    override CPPFLAGS += -DOPENACC -DGPU
  endif
  ifeq ($(GPU_OPENACC_UM),y)
    override CPPFLAGS += -DGPU_OPENACC_UM -DGPU
  endif
  ifeq ($(GPU_CUDA_UM),y)
    NVCCFLAGS = -ccbin=$(CXX)
    override CPPFLAGS += -DGPU_CUDA_UM -DGPU
  endif
endif

ifeq ($(USE_OPENMP_SCHEDULE),y)
  override CPPFLAGS += -DUSE_OPENMP_SCHEDULE
endif
ifeq ($(USE_INLINE_MATH),y)
  override CPPFLAGS += -DUSE_INLINE_MATH
endif

CDLDFLAGS=-lstdc++ -L$(dir $(shell which nvcc))../lib64 -lcudart

#
# IBM XL COMPILER - xlc_r
#
ifeq ($(COMP_CHOICE),xl)
  CFLAGS += -Ofast

  ifeq ($(OPENMP),y)
    CFLAGS += -qsmp=omp:noauto
  else ifeq ($(OPENMP_TARGET),y)
    CFLAGS += -qsmp=omp:noauto
  endif

  # Compiler flags for different platforms
  ifeq ($(LMOD_SYSTEM_NAME),summitdev)
    CFLAGS += -qarch=pwr8 -qtune=pwr8
  else ifeq ($(LMOD_SYSTEM_NAME),summit)
    CFLAGS += -qarch=pwr9 -qtune=pwr9
  endif

  # Explicitly turn off with -qnooffload. This seems to execute with 1 thread though
  ifeq ($(GPU),y)
    CFLAGS += -qoffload
    ifeq ($(LMOD_SYSTEM_NAME),summitdev)
       CFLAGS += -qtgtarch=sm_60
    else ifeq ($(LMOD_SYSTEM_NAME),summit)
       CFLAGS += -qtgtarch=sm_70
    endif
  endif

  ifeq ($(VERBOSE),y)
    CFLAGS += -qreport
    ifeq ($(GPU),y)
      CFLAGS += -Xptxas -v
    endif
  endif

  FCFLAGS := $(CFLAGS)
  CFLAGS += -fstrict-aliasing -Werror -Wall -Wno-unused-variable
  CXXFLAGS += $(CFLAGS) -std=c++11

#
# LLVM COMPILER - clang
#
else ifeq ($(COMP_CHOICE),clang)

  # -O3 and -Ofast cause a deadlock with Clang-7.0.0
  CFLAGS += -O2 -ffast-math -ffp-contract=fast -fstrict-aliasing -Werror -Wall -Wno-unused-variable
  LDFLAGS += -lm

  ifeq ($(OPENMP),y)
    CFLAGS += -fopenmp
  else ifeq ($(OPENMP_TARGET),y)
    CFLAGS += -fopenmp
  endif

  # Compiler flags for different platforms
  ifeq ($(LMOD_SYSTEM_NAME),summitdev)
    CFLAGS += --target="powerpc64le-unknown-linux-gnu"
  else ifeq ($(LMOD_SYSTEM_NAME),summit)
    CFLAGS += --target="powerpc64le-unknown-linux-gnu"
  endif

  ifeq ($(GPU),y)
    # clang-ykt option: -fopenmp-nonaliased-maps
    CFLAGS += -fopenmp-targets=nvptx64-nvidia-cuda --cuda-path=${CUDA_PATH} -ffp-contract=fast

    # Arch specific flags: llvm/tools/clang/test/Driver/openmp-offload-gpu.c
    ifeq ($(LMOD_SYSTEM_NAME),summitdev)
      CFLAGS += -Xopenmp-target=nvptx64-nvidia-cuda -march=sm_60
    else ifeq ($(LMOD_SYSTEM_NAME),summit)
      CFLAGS += -Xopenmp-target=nvptx64-nvidia-cuda -march=sm_70
    else
      # Macro defines used for bug https://bugs.llvm.org/show_bug.cgi?id=38464 - only needed on x86
      CFLAGS += -D__NO_MATH_INLINES -U__SSE2_MATH__ -U__SSE_MATH__
    endif
  endif

  ifeq ($(VERBOSE),y)
    # clang-ykt option: -Rpass-analysis (shows "Combined construct detected.")
    # -Rpass=.* -Rpass=inline
    CFLAGS += -Rpass-analysis
    ifeq ($(GPU),y)
      CFLAGS += -Xcuda-ptxas -v -Wno-unused-command-line-argument
    endif
  endif

  CXXFLAGS += $(CFLAGS) -std=c++11
  FCFLAGS += $(CFLAGS)

#
# PGI COMPILER - pgcc
#
else ifeq ($(COMP_CHOICE),pgi)

  CFLAGS += -fast
  ifeq ($(OPENMP),y)
    CFLAGS += -mp
  else ifeq ($(OPENMP_TARGET),y)
    CFLAGS += -mp
  else ifeq ($(OPENACC),y)
    CFLAGS += -acc
  endif

  ifeq ($(GPU),y)
    ifeq ($(OPENACC),y)
      tgtflags=tesla:fastmath

      ifeq ($(GPU_OPENACC_UM),y)
        tgtflags:=$(tgtflags),managed
      endif
      ifeq ($(VERBOSE),y)
        tgtflags:=$(tgtflags),ptxinfo
      endif
      ifeq ($(LMOD_SYSTEM_NAME),summitdev)
        tgtflags:=$(tgtflags),cc60,cuda9.0
      else ifeq ($(LMOD_SYSTEM_NAME),summit)
        tgtflags:=$(tgtflags),cc70,cuda9.2
      else ifeq ($(HOSTNAME),psgcluster)
        tgtflags:=$(tgtflags),cc70,cuda9.1
      else ifeq ($(SLURM_CLUSTER_NAME),escori)
        tgtflags:=$(tgtflags),cc70,cuda9.2
      endif

      CFLAGS += -ta=$(tgtflags)
    endif
  else
    ifeq ($(OPENACC),y)
      CFLAGS += -ta=multicore
    endif
  endif

  ifeq ($(VERBOSE),y)
    ifeq ($(OPENMP),y)
      CFLAGS += -Minfo=mp
    else ifeq ($(OPENACC),y)
      CFLAGS += -Minfo=accel
    endif
  endif

  CXXFLAGS += $(CFLAGS) -std=c++11
  FCFLAGS += $(CFLAGS)

#
# CUDA COMPILER - nvcc
#
else ifeq ($(COMP_CHOICE),nvcc)
  ifeq ($(LMOD_SYSTEM_NAME),summitdev)
    cudafalgs:=-arch=sm_60
  else ifeq ($(LMOD_SYSTEM_NAME),summit)
    cudaflags:=-arch=sm_70
  else ifeq ($(HOSTNAME),psgcluster)
    cudaflags:=-arch=sm_70
  else ifeq ($(SLURM_CLUSTER_NAME),escori)
    cudaflags:=-arch=sm_70
  endif
  CFLAGS += -O3 -std=c++11 -Wno-deprecated-gpu-targets $(cudaflags)
  CFLAGS += -I$(CUDA_PATH)/samples/common/inc/
  CXXFLAGS += $(CFLAGS)

#
# Kokkos COMPILER - nvcc_wrapper
#
else ifeq ($(COMP_CHOICE),kokkos)
  ifeq ($(LMOD_SYSTEM_NAME),summitdev)
    cudafalgs:=-arch=sm_60
  else ifeq ($(LMOD_SYSTEM_NAME),summit)
    cudaflags:=-arch=sm_70
  else ifeq ($(HOSTNAME),psgcluster)
    cudaflags:=-arch=sm_70
  else ifeq ($(SLURM_CLUSTER_NAME),escori)
    cudaflags:=-arch=sm_70
  endif
  CFLAGS += -O3 -std=c++11 -Wno-deprecated-gpu-targets $(cudaflags)
  CFLAGS += -I$(CUDA_PATH)/samples/common/inc/
  CXXFLAGS += $(CFLAGS)

#
# INTEL COMPILER - icc
#
else ifeq ($(COMP_CHOICE),intel)

  # KNL: -xMIC-AVX512
  CFLAGS += -Ofast
  ifeq ($(OPENMP),y)
    CFLAGS += -qopenmp
    ifeq ($(CRAY_CPU_TARGET),haswell)
        CFLAGS += -xcore-avx2
    else ifeq ($(CRAY_CPU_TARGET),mic-knl)
        CFLAGS += -xmic-avx512
    endif
  else ifeq ($(OPENMP_TARGET),y)
    CFLAGS += -qopenmp-offload=host
  endif

  CXXFLAGS += $(CFLAGS) -std=c++11
  FCFLAGS += $(CFLAGS)

#
# GNU COMPILER - gcc
#
else ifeq ($(COMP_CHOICE),gcc)

  CFLAGS += -Ofast -fstrict-aliasing
  ifeq ($(OPENMP),y)
    CFLAGS += -fopenmp
  else ifeq ($(OPENMP_TARGET),y)
    CFLAGS += -fopenmp
  else ifeq ($(OPENACC),y)
    CFLAGS += -fopenacc
  endif

  ifeq ($(GPU),y)
    CFLAGS += -foffload=nvptx-none="-Ofast -lm -misa=sm_35"
  else
    ifeq ($(OPENMP_TARGET),y)
      CFLAGS += -foffload=disable
    endif
  endif

  # Compiler flags for different platforms
  ifeq ($(LMOD_SYSTEM_NAME),summitdev)
    CFLAGS += -mcpu=power8 -mtune=power8
  else ifeq ($(LMOD_SYSTEM_NAME),summit)
    CFLAGS += -mcpu=power9 -mtune=power9
  endif

  ifeq ($(VERBOSE),y)
    ifeq ($(GPU),y)
      CFLAGS += -fopt-info-all-omp -fopt-info-vec
    endif
  endif

  CXXFLAGS += $(CFLAGS) -std=c++11
  FCFLAGS += $(CFLAGS)
endif

ifeq ($(DEBUG),y)
  CXXFLAGS += -g
  FCFLAGS += -g
endif

CXXFLAGS += $(USER_CXXFLAGS)
