#include "instr.h"


e_opcode instr_opcode (uint32_t instr) {
    return instr >> 24;
}


e_register instr_first_operand (uint32_t instr) {
    return (instr >> 21) & 0x7;
}


e_addr_mode instr_addr_mode (uint32_t instr) {
    return (instr >> 19) & 0x3;
}


e_register instr_index_reg (uint32_t instr) {
    return (instr >> 16) & 0x7;
}


uint16_t instr_addr (uint32_t instr) {
    return instr;
}


uint32_t make_instr (
        e_opcode opcode,
        e_register first_operand,
        e_addr_mode addr_mode,
        e_register index_reg,
        uint16_t addr)
{
    return (opcode << 24) | (first_operand << 21) | (addr_mode << 19) | (index_reg << 16) | addr;
}

