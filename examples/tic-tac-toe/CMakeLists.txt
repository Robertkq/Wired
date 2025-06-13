cmake_minimum_required(VERSION 3.22)
project(chat VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find Wired package
find_package(Wired REQUIRED)
message("Wired_INCLUDE_DIRS: ${Wired_INCLUDE_DIRS}")

# Find OpenSSL package
find_package(OpenSSL REQUIRED)

# define client target with Wired as dependency
file(GLOB CLIENT_SRC src/client/*.cpp src/common/*.cpp)
add_executable(client ${CLIENT_SRC})
target_include_directories(client PRIVATE ./include/client)
target_include_directories(client PRIVATE ./include/common)
target_include_directories(client PRIVATE ${Wired_INCLUDE_DIRS})

target_link_libraries(client PRIVATE OpenSSL::SSL OpenSSL::Crypto)

set_target_properties(client PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
)

# define server target with Wired as dependency
file(GLOB SERVER_SRC src/server/*.cpp)
add_executable(server ${SERVER_SRC})
target_include_directories(server PRIVATE ./include/server)
target_include_directories(server PRIVATE ./include/common)
target_include_directories(server PRIVATE ${Wired_INCLUDE_DIRS})

target_link_libraries(server PRIVATE OpenSSL::SSL OpenSSL::Crypto)

set_target_properties(server PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
)

if (WIN32)
    target_compile_definitions(client PRIVATE _WIN32_WINNT=0x0601)
    target_compile_definitions(server PRIVATE _WIN32_WINNT=0x0601)
endif()