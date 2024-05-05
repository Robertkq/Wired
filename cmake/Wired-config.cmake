file(GLOB_RECURSE HEADERS "include/*.h")

message("list_dir:   ${CMAKE_CURRENT_LIST_DIR}")
message("current dir: ${CMAKE_CURRENT_SOURCE_DIR}")

add_library(Wired INTERFACE ${HEADERS})

target_include_directories(Wired INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../include)

message("Sucessfully added Wired library to project.")
