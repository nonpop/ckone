cmake_minimum_required(VERSION 2.8)
project(ckone)

###################################
# Build-time configurable options #
###################################

# The default emulator memory size (in 4-byte words)
set (DEFAULT_MEMORY_SIZE 128)

# The default number of columns in memory dumps
# The number of characters per row is 12*(1 + columns)
set (DEFAULT_MEMDUMP_COLUMNS 8)

# The default numeric base for memory dumps.
# Only 10 and 16 are supported.
set (DEFAULT_MEMDUMP_BASE 10)

# End of build-time configurable options
###################################


# Program version; do not edit this
set (VERSION 0.9)


set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

configure_file (
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
    )

include_directories("${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/src")

set(CMAKE_C_FLAGS "-std=c99 -Wall -Wextra -Werror -pedantic -g3")


add_library(emu STATIC src/alu.c src/args.c src/cpu.c src/ext.c src/instr.c src/log.c src/mmu.c)
add_executable(ckone src/ckone.c src/symtable.c src/main.c)
target_link_libraries(ckone emu)
add_executable(ckone_tests test/main.c test/test_alu.c test/test_cpu.c test/test_instr.c test/test_mmu.c test/util.c)
target_link_libraries(ckone_tests emu)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating documentation with Doxygen" VERBATIM
        )
endif(DOXYGEN_FOUND)

