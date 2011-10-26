#include <stdbool.h>
#include <stdint.h>


typedef struct {
    char* stdin_file;
    char* stdout_file;
    size_t mem_size;
    int32_t mmu_base;
    int32_t mmu_limit;
    bool step;
    bool verbose;
    bool emulate_bugs;
    char* program;
} s_arguments;


extern s_arguments args;

