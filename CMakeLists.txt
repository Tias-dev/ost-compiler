cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDART 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(
  OST_compiler
  LANGUAGES CXX
)

file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES FALSE ./src/*.cpp)
file(GLOB_RECURSE TESTS_FILES LIST_DIRECTORIES FALSE ./tests/*.cpp)

include_directories("include")
add_compile_options(-g -std=c++20)

add_library(src_lib STATIC ${SRC_FILES})

add_executable(main main.cpp)
target_link_libraries(main src_lib)

# include(FetchContent)
# FetchContent_Declare(
#   googletest
#   GIT_REPOSITORY https://github.com/google/googletest.git
#   GIT_TAG        release-1.11.0
# )
#
# # For Windows: Prevent overriding the parent project's compiler/linker settings
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)
#
# add_executable(tests ${TESTS_FILES})
# target_link_libraries(tests GTest::gtest_main src_lib)
#
# include(GoogleTest)
# gtest_discover_tests(tests)


