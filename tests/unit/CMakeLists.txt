cmake_minimum_required(VERSION 3.22)
project(unit_tests)

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
    "src/sanity.cpp"
    "src/client_server_tests.cpp")

add_executable(unit ${UNIT_TEST_SOURCES})

target_include_directories(unit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(unit PRIVATE -DASIO_STANDALONE)
target_link_libraries(unit PRIVATE GTest::gtest)

find_package(OpenSSL REQUIRED)
find_package(wired REQUIRED)

target_link_libraries(unit PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_include_directories(unit PRIVATE ${Wired_INCLUDE_DIRS})

set_target_properties(unit PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_options(unit PRIVATE --coverage)
target_link_libraries(unit PRIVATE --coverage)

if(WIN32)
    target_compile_definitions(unit PRIVATE _WIN32_WINNT=0x0601)
    target_compile_options(unit PRIVATE /await:strict /std:c++20 /W4)
else()
    target_compile_options(unit PRIVATE -std=c++20 -pthread -Wall)
endif()