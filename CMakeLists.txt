cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(
  OST_compiler
  LANGUAGES CXX
)

file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES FALSE ./src/*.cpp)
file(GLOB_RECURSE TESTS_FILES LIST_DIRECTORIES FALSE ./tests/*.cpp)

include_directories("include")

add_library(src_lib STATIC ${SRC_FILES})

add_executable(ost Compiler.cpp)
target_link_libraries(ost src_lib)

add_executable(tu4run Tu4Run.cpp)
target_link_libraries(tu4run src_lib)

add_executable(osttest Tests.cpp)
target_link_libraries(osttest src_lib)

add_executable(ostdb Debugger.cpp)
target_link_libraries(ostdb src_lib)

add_executable(ostbuild Builder.cpp)
target_link_libraries(ostbuild src_lib)

