#include "test.h"
#include "../src/instr.h"


void test_instr () {
    {
        TEST_STR ("JZER", instr_name (JZER));
    }
    {
        uint32_t instr = make_instr (LOAD, R2, DIRECT, R1, 1234);
        TEST (e_opcode, "0x%x", LOAD, instr_opcode (instr));
        TEST (e_register, "%d", R2, instr_first_operand (instr));
        TEST (e_addr_mode, "%d", DIRECT, instr_addr_mode (instr));
        TEST (e_register, "%d", R1, instr_index_reg (instr));
        TEST (uint16_t, "%d", 1234, instr_addr (instr));
    }
    {
        uint32_t instr = 52428801;
        TEST (e_opcode, "0x%x", IN, instr_opcode (instr));
        TEST (e_register, "%d", R1, instr_first_operand (instr));
        TEST (e_addr_mode, "%d", IMMEDIATE, instr_addr_mode (instr));
        TEST (e_register, "%d", R0, instr_index_reg (instr));
        TEST (uint16_t, "%d", 1, instr_addr (instr));
    }
}

