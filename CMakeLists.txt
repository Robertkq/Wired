cmake_minimum_required(VERSION 3.2)
project(Wired VERSION 1.0.0)

file(GLOB_RECURSE HEADERS "include/*.h")

add_library(Wired INTERFACE ${HEADERS})

target_include_directories(Wired INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(Wired INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/asio)

target_compile_features(Wired INTERFACE cxx_std_20)

install(TARGETS Wired DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
install(FILES cmake/wired-config.cmake DESTINATION cmake)
install(DIRECTORY vendor/asio/asio/include/ DESTINATION include/asio)

