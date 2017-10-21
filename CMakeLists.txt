cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -fopenmp -O2")

project(SimpleDataBase)
include_directories(include)

add_executable(testfilesystem unittests/testfilesystem.cpp)
add_executable(testrecordinsert src/recordman.cpp unittests/testrecordinsert.cpp)
