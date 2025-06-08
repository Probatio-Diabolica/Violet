cmake_minimum_required(VERSION 3.28)
project(Violet LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE HEADER_FILES
    include/*.hpp
)
file(GLOB_RECURSE SOURCE_FILES
    src/main.cc
    src/*.cc
)

add_executable(violet ${SOURCE_FILES})

target_include_directories(violet PRIVATE include)

target_compile_options(violet PRIVATE -O3 -march=native -Wall -Wextra)
