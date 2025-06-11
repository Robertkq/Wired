cmake_minimum_required(VERSION 3.22)
project(static_analyzer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/wired/*.h
)

message(STATUS "Header files to analyze:")
foreach(file ${HEADER_FILES})
    message(STATUS "  ${file}")
endforeach()

add_custom_target(static_analysis_target ALL
    COMMENT "Running Cppcheck on header files"
)

find_program(CPPCHECK cppcheck)
if(CPPCHECK)
    foreach(file ${HEADER_FILES})
        add_custom_command(
            TARGET static_analysis_target
            COMMAND ${CPPCHECK} --enable=all --std=c++20 --language=c++ --suppress=missingInclude --suppress=unusedFunction --suppress=missingIncludeSystem --suppress=operatorEqVarError ${file}
            COMMENT "Running Cppcheck on ${file}"
        )
    endforeach()
else()
    message(WARNING "Cppcheck not found. Static analysis will not run.")
endif()