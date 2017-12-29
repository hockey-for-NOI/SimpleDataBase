cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

project(SimpleDataBase)
include_directories(include include/external/hsql)

list(APPEND SRC_LIST src/parser/bison_parser.cpp)
list(APPEND SRC_LIST src/parser/destruct.cpp)
list(APPEND SRC_LIST src/parser/Expr.cpp)
list(APPEND SRC_LIST src/parser/flex_lexer.cpp)
list(APPEND SRC_LIST src/parser/SQLParser.cpp)
list(APPEND SRC_LIST src/parser/SQLParserResult.cpp)

list(APPEND SRC_LIST src/utils.cpp)

list(APPEND SRC_LIST src/recordman.cpp)
list(APPEND SRC_LIST src/record_insert.cpp)
list(APPEND SRC_LIST src/record_remove.cpp)
list(APPEND SRC_LIST src/record_access.cpp)

list(APPEND SRC_LIST src/indexman.cpp)
list(APPEND SRC_LIST src/index_insert.cpp)
list(APPEND SRC_LIST src/index_remove.cpp)
list(APPEND SRC_LIST src/index_getobj.cpp)

list(APPEND SRC_LIST src/area.cpp)
list(APPEND SRC_LIST src/systemdb.cpp)
list(APPEND SRC_LIST src/translate.cpp)

#add_executable(testfilesystem ${SRC_LIST} unittests/testfilesystem.cpp)

#add_executable(testrecordinsert ${SRC_LIST} unittests/testrecordinsert.cpp)
#add_executable(testrecordremove ${SRC_LIST} unittests/testrecordremove.cpp)
#add_executable(testrecordaccess ${SRC_LIST} unittests/testrecordaccess.cpp)

#add_executable(testindexinsert ${SRC_LIST} unittests/testindexinsert.cpp)
#add_executable(testindexremove ${SRC_LIST} unittests/testindexremove.cpp)
#add_executable(testindexgetobj ${SRC_LIST} unittests/testindexgetobj.cpp)

add_executable(SimpleDataBase ${SRC_LIST} src/main.cpp)
