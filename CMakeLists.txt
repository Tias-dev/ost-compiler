cmake_minimum_required(VERSION 3.10)

project(
  OST_compiler
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDART 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("include")
add_compile_options(-g)

add_executable(main main.cpp)

