cmake_minimum_required(VERSION 2.8)
project(ckone)

###################################
# Build-time configurable options #
###################################

# The default emulator memory size (in 4-byte words)
set (DEFAULT_MEMORY_SIZE 64)

# The default number of columns in memory dumps
set (DEFAULT_MEMDUMP_COLUMNS 4)

# End of build-time configurable options
###################################


# Program version; do not edit this
set (VERSION 0.9)


set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

configure_file (
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
    )

include_directories("${PROJECT_BINARY_DIR}")

set(CMAKE_C_FLAGS "-std=c99 -Wall -Wextra -Werror -pedantic -g3")

add_subdirectory(src)
add_subdirectory(test)

