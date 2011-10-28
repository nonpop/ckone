#ifndef ARGS_H
#define ARGS_H


#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>


/**
 * A structure containing all the variables which can be set
 * by command line arguments.
 */
typedef struct {
    char* stdin_file;       ///< The file where the STDIN device gets its data from.
    char* stdout_file;      ///< The file where the STDOUT device writes its data to.
    size_t mem_size;        ///< The size of the emulator memory, in words (1 word = 4 bytes).
    int32_t mmu_base;       ///< The value of the MMU_BASE register (s_ckone::mmu_base).
    int32_t mmu_limit;      ///< The value of the MMU_LIMIT register (s_ckone::mmu_limit).
    bool clean;             ///< Whether the memory and registers should be zeroed before emulation begins.
    int mem_cols;           ///< How many columns to print in the memory dumps.
    bool step;              ///< If true, the emulator will pause after each instruction.
    int verbosity;          ///< If 0, only the most important messages are shown. If 2, every debug message is shown.
    bool emulate_bugs;      ///< If true, emulate bugs found in TitoKone 1.203.
    char* program;          ///< The file where the program is to be read from. If "-", stdin is used.
    bool include_symtable;  ///< If true, the symbol table is printed in every dump.
} s_arguments;


extern s_arguments args;


#endif

