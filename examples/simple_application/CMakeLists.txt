cmake_minimum_required(VERSION 3.22)
project(simple_server VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find Wired package
find_package(Wired REQUIRED)

message("Wired_INCLUDE_DIRS: ${Wired_INCLUDE_DIRS}")

# define client target with Wired as dependency
add_executable(client src/client.cpp)
target_include_directories(client PRIVATE include)
target_include_directories(client PRIVATE ${Wired_INCLUDE_DIRS})

set_target_properties(client PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
)

# define server target with Wired as dependency
add_executable(server src/server.cpp)
target_include_directories(server PRIVATE include)
target_include_directories(server PRIVATE ${Wired_INCLUDE_DIRS})

set_target_properties(server PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
)

if (WIN32)
    target_compile_definitions(client PRIVATE _WIN32_WINNT=0x0601)
    target_compile_definitions(server PRIVATE _WIN32_WINNT=0x0601)
endif()