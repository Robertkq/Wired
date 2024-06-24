cmake_minimum_required(VERSION 3.2)
project(WiredUnitTests VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE UNIT_TEST_SOURCES 
    "src/main.cpp"
    "src/message_tests.cpp"
    "src/connection_tests.cpp"
    "src/sanity.cpp")

add_executable(unit_tests ${UNIT_TEST_SOURCES})
target_link_libraries(unit_tests PRIVATE GTest::gtest)

target_include_directories(unit_tests PRIVATE include)
target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/../../include)
target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/../../vendor/asio/asio/include)

set_target_properties(unit_tests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
)

if(WIN32)
    target_compile_definitions(unit_tests PRIVATE _WIN32_WINNT=0x0601)
endif()
