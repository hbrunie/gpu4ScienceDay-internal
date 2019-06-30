cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(STATUS "Error! Building from the source directory may overwrite Makefile")
    message(STATUS "Remove 'CMakeCache.txt' and 'CMakeFiles' and build is separate directory")
    message(FATAL_ERROR "In-source build")
endif()

project(timemory-connector LANGUAGES CXX)

include(CheckLanguage)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Build flags")
set(CMAKE_CXX_STANDARD 11 CACHE STRING "C++ language standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "Require language standard flags")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(ENABLE_CUDA "Enable CUDA support" OFF)
option(ENABLE_PAPI "Enable PAPI support" OFF)
option(ENABLE_CUPTI "Enable CUPTI support" OFF)

if(USE_CUDA)
    check_language(CUDA)
    if(CMAKE_CUDA_COMPILER)
        enable_language(CUDA)
    else()
        message(FATAL_ERROR "CUDA compiler not found. USE_CUDA=ON ignored")
    endif()
endif()

# CMake INTERFACE target for timemory that provides include path
set(COMPONENTS headers)

# process options
foreach(_TYPE CUDA PAPI CUPTI)
    if(ENABLE_${_TYPE})
        string(TOLOWER "${_TYPE}" _COMP)
        list(APPEND COMPONENTS ${_COMP})
        if(NOT "shared" IN_LIST COMPONENTS)
            list(APPEND COMPONENTS shared cxx)
        endif()
    endif()
endforeach()

# find the timemory package
find_package(TiMemory REQUIRED COMPONENTS ${COMPONENTS})

add_library(gpp_timemory SHARED gpp_timemory.cpp)
target_link_libraries(gpp_timemory timemory)
