include ../Makefile.inc

gpp_obj = gpp.o

CXXFLAGS += -I$(CUDA_ROOT)/include -I../external

all: gpp_timemory.so gpp
EXEC=gpp

gpp_timemory.so:
	g++ -shared -fPIC -o stubs/lib$@ stubs/gpp_timemory.cpp

gpp: $(gpp_obj)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(gpp_obj) -o $(EXEC).ex $(LINKER_PREFIX)-rpath=stubs -L$(PWD)/stubs -lgpp_timemory

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

gpp.cpp: ../external/ComplexClass/CustomComplex.h

clean:
	rm -f *.o *.lst $(EXEC).ex *.ptx *.cub stubs/*.so
