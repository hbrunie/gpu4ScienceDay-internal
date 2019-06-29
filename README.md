# BerkeleyGW-Kernels

Mini-apps from BerkeleyGW kernels. GPP and FF

Master branch - OpenMP3.0
OpenMP4.5 branch - OpenMP 4.5 versions with xl, gcc, clang, cray
OpenACC branch - OpenACC with pgi, cray, gcc
CUDA branch


Sequential Runtime :
./gpp.ex 512 2 32768 20
OpenMP 3.0
Number of OpenMP Threads = 64
Sizeof(CustomComplex<double> = 16 bytes
number_bands = 512       nvband = 2      ncouls = 32768  nodes_per_group  = 20   ngpown = 1638   nend = 3        nstart = 0
Memory Foot Print = 2.10048 GBs

 Final achtemp
 ( -264241151.997370, 1321205760.015211)
 ********** Kernel Time Taken **********= 220 secs
 ********** Total Time Taken **********= 225.71 secs
