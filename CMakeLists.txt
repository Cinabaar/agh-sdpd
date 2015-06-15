cmake_minimum_required(VERSION 2.7)
project(sdpd)

find_package(MPI REQUIRED)

set(CMAKE_CXX_FLAGS -std=c++11)
set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(${CMAKE_SOURCE_DIR}/src/sdpd)
#add_subdirectory(${CMAKE_SOURCE_DIR}/src/rend)
#add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/glew-cmake)

add_library(cppformat STATIC ${CMAKE_SOURCE_DIR}/3rdparty/cppformat/format.cc)
