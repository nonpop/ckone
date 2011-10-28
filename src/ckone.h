/**
 * @file ckone.h
 *
 * The main state structure.
 */

#ifndef CKONE_H
#define CKONE_H


/**
 * The ckone state structure. The contents of this structure
 * define the state of the emulator completely.
 */
typedef struct {
    int32_t r[8];               ///< The working registers R0 to R7.
    
    int32_t alu_in1;            ///< The first ALU operand register.
    int32_t alu_in2;            ///< The second ALU operand register.
    int32_t alu_out;            ///< The ALU result register.

    int32_t tr;                 ///< The temporary register. Used for various things.
    int32_t pc;                 ///< The program counter. Points to the next instruction in memory.
    int32_t ir;                 ///< The instruction register. Contains the currently executing instruction.
    int32_t sr;                 ///< The status register. See ::e_status_bits.

    int32_t mmu_base;           ///< The MMU base register. This is always added 
                                ///< to the address register before accessing the memory.

    int32_t mmu_limit;          ///< The MMU limit register. Tells how many words from
                                ///< mmu_base can be accessed by the program.

    int32_t mar;                ///< The memory address register.
    int32_t mbr;                ///< The memory buffer register.


    size_t mem_size;            ///< The size of the memory array in words (4-byte integers)
    int32_t* mem;               ///< The memory array.

    bool halted;                ///< True if the machine has halted.
} s_ckone;


/**
 * The working registers. These can be used as indices into s_ckone::r.
 */
typedef enum {
    R0 = 0, R1, R2, R3, R4, R5, R6, R7, 
    SP = 6,     ///< An alias for R6
    FP = 7      ///< An alias for R7
} e_register;


/**
 * The status register bits.
 */
enum e_status_bits {
    SR_G = 1 << 31,         ///< The first operand was greater than the second in a COMP.
    SR_E = 1 << 30,         ///< The operands were equal in a COMP.
    SR_L = 1 << 29,         ///< The first operand was less than the second in a COMP.
    SR_O = 1 << 28,         ///< The result of an arithmetic operation did not fit into an integer.
    SR_Z = 1 << 27,         ///< A division by zero has occurred.
    SR_U = 1 << 26,         ///< An unknown instruction opcode was encountered.
    SR_M = 1 << 25,         ///< The program tried to access memory beyond its limits. Trying to
                            ///< access an invalid device is also counted in this.
    SR_I = 1 << 24,         ///< Device interrupt (unused)
    SR_S = 1 << 23,         ///< Supervisor call (unused)
    SR_P = 1 << 22,         ///< Priviledged mode (unused)
    SR_D = 1 << 21,         ///< Interrupts disabled (unused)
};


#endif

