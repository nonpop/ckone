#include <stdio.h>
#include "instr.h"
#include "ckone.h"


typedef struct {
    e_opcode opcode;
    const char* name;
} s_instr_name;


#define I(instr) { instr, #instr }

s_instr_name instruction_names[] = {
    I(NOP),
    I(STORE), I(LOAD), I(IN), I(OUT),
    I(ADD), I(SUB), I(MUL), I(DIV), I(MOD),
    I(AND), I(OR), I(XOR), I(SHL), I(SHR), I(NOT), I(SHRA),
    I(COMP),
    I(JUMP), I(JNEG), I(JZER), I(JPOS), I(JNNEG), I(JNZER), I(JNPOS),
    I(JLES), I(JEQU), I(JGRE), I(JNLES), I(JNEQU), I(JNGRE),
    I(CALL), I(EXIT), I(PUSH), I(POP), I(PUSHR), I(POPR),
    I(SVC),
    { NOP, NULL }
};


const char* op_name (e_opcode opcode) {
    s_instr_name* in = instruction_names;
    while (in->name) {
        if (in->opcode == opcode)
            return in->name;

        in++;
    }

    return "(Unknown)";
}


e_opcode instr_opcode (int32_t instr) {
    return instr >> 24;
}


e_register instr_first_operand (int32_t instr) {
    return (instr >> 21) & 0x7;
}


e_addr_mode instr_addr_mode (int32_t instr) {
    return (instr >> 19) & 0x3;
}


e_register instr_index_reg (int32_t instr) {
    return (instr >> 16) & 0x7;
}


int16_t instr_addr (int32_t instr) {
    return instr;
}


int32_t make_instr (
        e_opcode opcode,
        e_register first_operand,
        e_addr_mode addr_mode,
        e_register index_reg,
        int16_t addr)
{
    return (opcode << 24) | (first_operand << 21) | (addr_mode << 19) | (index_reg << 16) | addr;
}


static char* reg_name (e_register r) {
    static char buf[3];

    if (r < R6)
        snprintf (buf, 3, "R%d", r);
    else if (r == SP)
        snprintf (buf, 3, "SP");
    else if (r == FP)
        snprintf (buf, 3, "FP");

    return buf;
}


void instr_string (uint32_t instr, char* buffer, size_t buf_size) {
    snprintf (buffer, buf_size, "%s, first opr: %s, indirections: %u, "
            "index: %s, constant: 0x%04x (%d)",
            op_name (instr_opcode (instr)), reg_name (instr_first_operand (instr)),
            instr_addr_mode (instr), reg_name (instr_index_reg (instr)),
            instr_addr (instr), instr_addr (instr));
}

