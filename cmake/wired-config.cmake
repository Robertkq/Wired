file(GLOB_RECURSE HEADERS "include/*.h")

add_library(Wired INTERFACE ${HEADERS})

target_include_directories(Wired INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../include)
target_include_directories(Wired INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../include/vendor/asio)

message("Sucessfully added Wired library to project.")
