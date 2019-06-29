include ../../../Makefile.inc


#gpp_obj = test_TeamsThreads.o
gpp_obj = gpp.o
gpp_cudaobj = gpp_CUDAKernel.o

CXXFLAGS += -I$(CUDA_ROOT)/include

all: gpp
EXEC=gpp

gpp: $(gpp_obj) $(gpp_cudaobj)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(gpp_obj) $(gpp_cudaobj) -o $(EXEC).ex

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.cu
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

gpp.cpp: ../../../ComplexClass/CustomComplex.h

clean:
	rm -f *.o *.lst $(EXEC).ex *.ptx *.cub
