cmake_minimum_required(VERSION 3.27)
project(simple_server VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find Wired package
find_package(Wired REQUIRED)

# define client target with Wired as dependency
add_executable(client src/client.cpp)
target_link_libraries(client PRIVATE Wired)
target_include_directories(client PRIVATE include)

# define server target with Wired as dependency
add_executable(server src/server.cpp)
target_link_libraries(server PRIVATE Wired)
target_include_directories(server PRIVATE include)

set_target_properties(client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release
)

set_target_properties(server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release
)

if (WIN32)
    target_compile_definitions(client PRIVATE _WIN32_WINNT=0x0601)
    target_compile_definitions(server PRIVATE _WIN32_WINNT=0x0601)
endif()