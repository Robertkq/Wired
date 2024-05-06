file(GLOB_RECURSE HEADERS "include/*.h")

find_package(asio REQUIRED)

target_include_directories(Wired INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../include)
target_include_directories(Wired INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../vendor/asio/asio/include)

message("Sucessfully added Wired library to project.")
