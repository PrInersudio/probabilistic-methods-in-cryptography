cmake_minimum_required(VERSION 3.15...4.0)
project(CryptographicProtocols LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (WARNING_FLAGS
    -Werror
    -pedantic-errors

    -Wall
    -Wextra
    -Wpedantic

    -Wcast-align
    -Wcast-qual
    -Wconversion
    -Wctor-dtor-privacy
    -Wduplicated-branches
    -Wduplicated-cond
    -Wextra-semi
    -Wfloat-equal
    -Wlogical-op
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Woverloaded-virtual
    -Wredundant-decls
    -Wsign-conversion
    -Wsign-promo
)
add_compile_options(${WARNING_FLAGS} -O3 -fconcepts-diagnostics-depth=2)

find_package(TBB REQUIRED)

add_library(Kuznechik OBJECT Kuznechik.cpp)
add_library(LabsDefinitions OBJECT LabsDefinitions.cpp)
add_library(SubstituteGenerator OBJECT SubstituteGenerator.cpp)
add_library(Lab1 OBJECT Lab1.cpp)
add_library(Lab2 OBJECT Lab2.cpp)
add_library(Lab3 OBJECT Lab3.cpp)

add_executable(labs main.cpp)
target_link_libraries(labs PRIVATE Kuznechik TBB::tbb SubstituteGenerator LabsDefinitions Lab1 Lab2 Lab3)
add_executable(BestApprox BestApprox.cpp)
target_link_libraries(BestApprox PRIVATE Kuznechik TBB::tbb SubstituteGenerator LabsDefinitions)