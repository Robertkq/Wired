cmake_minimum_required(VERSION 3.22)
project(unit_tests)

set(CMAKE_CXX_COMPILER "/usr/bin/g++")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(UNIT_TEST_SOURCES 
    "src/main.cpp"
    "src/message_tests.cpp"
    "src/connection_tests.cpp"
    "src/sanity.cpp")

add_executable(unit ${UNIT_TEST_SOURCES})

target_include_directories(unit PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_options(unit PRIVATE -std=c++20 -pthread -Wall -Wextra -Werror -Wpedantic -fcoroutines)
target_compile_definitions(unit PRIVATE -DASIO_STANDALONE)
target_link_libraries(unit PRIVATE GTest::gtest)

find_package(wired REQUIRED)

target_include_directories(unit PRIVATE ${wired_INCLUDE_DIRS})

set_target_properties(unit PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
)

if(WIN32)
    target_compile_definitions(unit PRIVATE _WIN32_WINNT=0x0601)
    target_compile_options(unit PRIVATE /await:strict)
endif()
