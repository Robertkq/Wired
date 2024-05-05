cmake_minimum_required(VERSION 3.16)
project(WiredPlayground VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Wired REQUIRED)

file(GLOB_RECURSE WIRED_PLAYGROUND_SOURCES src/*.cpp)

add_executable(wired_playground ${WIRED_PLAYGROUND_SOURCES})
target_link_libraries(my_project PRIVATE Wired)
