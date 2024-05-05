cmake_minimum_required(VERSION 3.16)
project(Wired VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(Wired INTERFACE ${HEADERS})

target_include_directories(Wired PUBLIC include)