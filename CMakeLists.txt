cmake_minimum_required(VERSION 3.21)
project(marty_decimal CXX)


if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


set(MODULE_ROOT "${CMAKE_CURRENT_LIST_DIR}")

file(GLOB_RECURSE sources "${MODULE_ROOT}/*.cpp")
list(FILTER sources EXCLUDE REGEX "pi_test.cpp$")
list(FILTER sources EXCLUDE REGEX "benchmark_tests.cpp$")
list(FILTER sources EXCLUDE REGEX "regression_tests.cpp$")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Sources" FILES ${sources})

file(GLOB_RECURSE headers "${MODULE_ROOT}/*.h")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Headers" FILES ${headers})

add_library(${PROJECT_NAME} OBJECT ${sources} ${headers})
add_library(marty::decimal ALIAS ${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN)

# target_include_directories(${PROJECT_NAME} PRIVATE ${MODULE_ROOT}/..)
