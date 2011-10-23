#ifndef INSTR_H
#define INSTR_H


#include <stdint.h>


typedef enum {
    NOP = 0x00,
    STORE = 0x01, LOAD, IN, OUT,
    ADD = 0x11, SUB, MUL, DIV, MOD,
    AND = 0x16, OR, XOR, SHL, SHR, NOT, SHRA,
    COMP = 0x1f,
    JUMP = 0x20, JNEG, JZER, JPOS, JNNEG, JNZER, JNPOS,
    JLES, JEQU, JGRE, JNLES, JNEQU, JNGRE,
    CALL = 0x31, EXIT, PUSH, POP, PUSHR, POPR,
    SVC = 0x70
} e_opcode;


typedef enum {
    R0 = 0, R1, R2, R3, R4, R5, R6, R7
} e_register;


typedef enum {
    IMMEDIATE = 0, DIRECT = 1, INDIRECT = 2,
} e_addr_mode;


e_opcode instr_opcode (uint32_t instr);
e_register instr_first_operand (uint32_t instr);
e_addr_mode instr_addr_mode (uint32_t instr);
e_register instr_index_reg (uint32_t instr);
uint16_t instr_addr (uint32_t instr);

uint32_t make_instr (
        e_opcode opcode,
        e_register first_operand,
        e_addr_mode addr_mode,
        e_register index_reg,
        uint16_t addr);


#endif

