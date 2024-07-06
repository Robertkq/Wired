cmake_minimum_required(VERSION 3.27)
project(Wired VERSION 1.0.0)

add_library(Wired INTERFACE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(UNIT_TESTS, "Build unit tests")
option(CODE_COVERAGE, "Run code coverage test TODO")
option(STATIC_ANALYSIS, "Run clang static analyzer TODO")
option(PLAYGROUND, "Build playground for experimental tests")

target_include_directories(Wired INTERFACE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(Wired INTERFACE ${CMAKE_SOURCE_DIR}/vendor/asio/asio/include)
target_compile_features(Wired INTERFACE cxx_std_20)

if(UNIT_TESTS)
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests/unit)
endif()

if(CODE_COVERAGE)
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests/code_coverage)
endif()

if(STATIC_ANALYZER)
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests/static_analyzer)
endif()

if(PLAYGROUND)
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests/playground)
endif()

install(TARGETS Wired DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
install(FILES cmake/wired-config.cmake DESTINATION cmake)
install(DIRECTORY vendor/asio/asio/include/ DESTINATION include/asio)

