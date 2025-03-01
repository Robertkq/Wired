cmake_minimum_required(VERSION 3.22)
project(Wired VERSION 1.0.0)

option(UNIT_TESTS, "Build unit tests")
option(CODE_COVERAGE, "Run code coverage test TODO")
option(STATIC_ANALYSIS, "Run clang static analyzer TODO")
option(PLAYGROUND, "Build playground for experimental tests")

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

# Install process

# Install the wired header files
install(DIRECTORY include/ DESTINATION wired/include)
install(DIRECTORY vendor/asio/asio/include DESTINATION wired/include/asio
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.ipp"
)
install(FILES cmake/wired-config.cmake DESTINATION cmake)

