cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

project(SimpleDataBase)
include_directories(include)

list(APPEND SRC_LIST src/utils.cpp)
list(APPEND SRC_LIST src/recordman.cpp)
list(APPEND SRC_LIST src/record_insert.cpp)
list(APPEND SRC_LIST src/record_remove.cpp)

add_executable(testfilesystem ${SRC_LIST} unittests/testfilesystem.cpp)
add_executable(testrecordinsert ${SRC_LIST} unittests/testrecordinsert.cpp)
add_executable(testrecordremove ${SRC_LIST} unittests/testrecordremove.cpp)
