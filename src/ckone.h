#ifndef CKONE_H
#define CKONE_H


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "log.h"


#define DEFAULT_MEM_SIZE 64
#define DEFAULT_MEM_COLS 4


typedef struct {
    int32_t r[8];              // registers R0-R7
    
    int32_t alu_in1, alu_in2;  // ALU registers
    int32_t alu_out;

    int32_t tr, pc, ir, sr;    // controller unit registers

    int32_t mmu_base, mmu_limit;   // MMU registers
    int32_t mar, mbr;

    size_t mem_size;
    int32_t* mem;

    bool halted;
} s_ckone;


typedef enum {
    R0 = 0, R1, R2, R3, R4, R5, R6, R7, SP = 6, FP = 7
} e_register;


// Status register bits
enum {
    SR_G = 1 << 31,     // Greater
    SR_E = 1 << 30,     // Equal
    SR_L = 1 << 29,     // Less
    SR_O = 1 << 28,     // Overflow
    SR_Z = 1 << 27,     // Division by zero
    SR_U = 1 << 26,     // Unknown instruction
    SR_M = 1 << 25,     // Forbidden memory address
    SR_I = 1 << 24,     // Device interrupt (unused)
    SR_S = 1 << 23,     // Supervisor call (unused)
    SR_P = 1 << 22,     // Priviledged mode (unused)
    SR_D = 1 << 21,     // Interrupts disabled (unused)
};


#endif

