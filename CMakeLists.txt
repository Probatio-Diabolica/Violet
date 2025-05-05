cmake_minimum_required(VERSION 3.14)
project(Voilet)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

file(GLOB SOURCES "src/*.cc")

add_executable(voilet ${SOURCES})
