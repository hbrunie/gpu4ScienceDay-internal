TiMemory_ROOT_DIR:=/usr/local
CXX=g++
CXXFLAGS=-O3 -std=c++11 -g -I$(TiMemory_ROOT_DIR)/include
SHARED_CXXFLAGS=-shared -fPIC

all: gpp_timemory.so

gpp_timemory.so: gpp_timemory.cpp
	$(CXX) $(SHARED_CXXFLAGS) $(CXXFLAGS) -o lib$@ gpp_timemory.cpp

format:
	clang-format -i *.hpp *.cpp

clean:
	rm -f *.so stub/*.so
