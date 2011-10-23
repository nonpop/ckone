#ifndef CKONE_H
#define CKONE_H


#include <stdint.h>
#include <stdbool.h>
#include "log.h"


typedef struct {
    uint32_t r[8];              // registers R0-R7
    
    uint32_t alu_in1, alu_in2;  // ALU registers
    uint32_t alu_out;

    uint32_t tr, pc, ir, sr;    // controller unit registers

    uint32_t mmu_base, mmu_limit;   // MMU registers
    uint32_t mar, mbr;

    uint32_t mem_size;
    uint32_t* mem;
} s_ckone;


// Status register bits
enum {
    SR_G = 1 << 31,     // Greater
    SR_E = 1 << 30,     // Equal
    SR_L = 1 << 29,     // Less
    SR_O = 1 << 28,     // Overflow
    SR_Z = 1 << 27,     // Division by zero
    SR_U = 1 << 26,     // Unknown instruction
    SR_M = 1 << 25,     // Forbidden memory address
    SR_I = 1 << 24,     // Device interrupt
    SR_S = 1 << 23,     // Supervisor call
    SR_P = 1 << 22,     // Priviledged mode
    SR_D = 1 << 21,     // Interrupts disabled
};


#endif

