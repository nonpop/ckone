cmake_minimum_required(VERSION 2.8)
project(ckone)
set (VERSION "0.9")

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set (CMAKE_VERBOSE_MAKEFILE true)

configure_file (
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
    )

include_directories("${PROJECT_BINARY_DIR}")

set(CMAKE_C_FLAGS "-std=c99 -Wall -Wextra -Werror -pedantic -g3")

add_subdirectory(src)
add_subdirectory(test)

