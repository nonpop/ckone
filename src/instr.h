#ifndef INSTR_H
#define INSTR_H


#include <stdint.h>
#include <stddef.h>
#include "ckone.h"


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
    IMMEDIATE = 0, DIRECT = 1, INDIRECT = 2,
} e_addr_mode;


//extern const char* op_name (e_opcode opcode);
extern e_opcode instr_opcode (int32_t instr);
extern e_register instr_first_operand (int32_t instr);
extern e_addr_mode instr_addr_mode (int32_t instr);
extern e_register instr_index_reg (int32_t instr);
extern int16_t instr_addr (int32_t instr);

extern int32_t make_instr (
        e_opcode opcode,
        e_register first_operand,
        e_addr_mode addr_mode,
        e_register index_reg,
        int16_t addr);

extern void instr_string (uint32_t instr, char* buffer, size_t buf_size);


#endif

