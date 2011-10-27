#ifndef ARGS_H
#define ARGS_H


#include <stdbool.h>
#include <stdint.h>


typedef struct {
    char* stdin_file;
    char* stdout_file;
    size_t mem_size;
    int32_t mmu_base;
    int32_t mmu_limit;
    bool clean;
    int mem_cols;
    bool step;
    int verbosity;
    bool emulate_bugs;
    char* program;
    bool include_symtable;
} s_arguments;


extern s_arguments args;


#endif

