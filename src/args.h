/**
 * @file args.h
 *
 * The structure which contains the options adjustable by
 * command line arguments.
 */

#ifndef ARGS_H
#define ARGS_H


/**
 * A structure containing all the variables which can be set
 * by command line arguments.
 */
typedef struct {
    /// The file where the STDIN device gets its data from.
    char* stdin_file;       

    /// The file where the STDOUT device writes its data to.
    char* stdout_file;      

    /// The size of the emulator memory, in words (1 word = 4 bytes).
    int32_t mem_size;       

    /// The value of the MMU_BASE register (s_ckone::mmu_base).
    int32_t mmu_base;       

    /// The value of the MMU_LIMIT register (s_ckone::mmu_limit).
    int32_t mmu_limit;      

    /// Whether the memory and registers should be zeroed before 
    /// emulation begins.
    bool zero;              

    /// How many columns to print in the memory dumps.
    int mem_cols;           

    /// Use the non-default base (10 or 16) in memory dumps.
    bool mem_swap_base;     

    /// If true, the emulator will pause after each instruction.
    bool step;              

    /// If 0, only the most important messages are shown. If 2, every 
    /// debug message is shown.
    int verbosity;          

    /// If true, emulate bugs found in Titokone 1.203.
    bool emulate_bugs;      

    /// The file where the program is to be read from. If "-", stdin is used.
    char* program;          

    /// If true, the symbol table is printed in every dump.
    bool include_symtable;  
} s_arguments;


extern s_arguments args;


#endif

