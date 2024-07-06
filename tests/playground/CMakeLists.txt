set(PLAYGROUND_SOURCES
    "src/main.cpp")

add_executable(playground ${PLAYGROUND_SOURCES})

target_include_directories(playground PRIVATE include)
target_link_libraries(playground PRIVATE Wired)

set_target_properties(playground PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
)

if(WIN32)
    target_compile_definitions(playground PRIVATE _WIN32_WINNT=0x0601)
endif()
