#include <string.h>
#include "test.h"
#include "util.h"
#include "../src/cpu.h"
#include "../src/instr.h"


extern void cpu_calculate_second_operand (s_ckone* kone);


void test_cpu () {
    s_ckone k;
    int32_t mem[4];

    k.mem = mem;
    k.mem_size = sizeof(mem)/sizeof(int32_t);

    {
        clear (&k);
        k.mmu_limit = 2;
        int32_t instr = make_instr (LOAD, R3, DIRECT, R1, 42);
        mem[1] = instr;

        TEST_I32X (0, k.ir);
        cpu_fetch_instr (&k);
        TEST_I32X (0, k.ir);
        TEST_I32X (0, k.sr & SR_M);
        cpu_fetch_instr (&k);
        TEST_I32X (instr, k.ir);
        TEST_I32X (0, k.sr & SR_M);
        cpu_fetch_instr (&k);
        TEST_I32X (SR_M, k.sr & SR_M);
    }
    {
        clear (&k);
        k.mmu_limit = 2;
        int32_t instr = make_instr (LOAD, R0, IMMEDIATE, R1, 42);
        mem[0] = instr;
        k.r[1] = 5;

        cpu_fetch_instr (&k);
        cpu_calculate_second_operand (&k);
        TEST_I32 (47, k.tr);
        TEST_I32X (0, k.sr & (SR_M | SR_U));
    }
    {
        clear (&k);
        int32_t instr = make_instr (LOAD, R0, DIRECT, R1, 2);
        mem[0] = instr;
        mem[3] = 42;
        k.r[1] = 1;

        cpu_fetch_instr (&k);
        cpu_calculate_second_operand (&k);
        TEST_I32 (42, k.tr);
        TEST_I32X (0, k.sr & (SR_M | SR_U));
    }
    {
        clear (&k);
        int32_t instr = make_instr (LOAD, R0, INDIRECT, R1, 2);
        mem[0] = instr;
        mem[1] = 1337;
        mem[3] = 1;
        k.r[1] = 1;

        cpu_fetch_instr (&k);
        cpu_calculate_second_operand (&k);
        TEST_I32 (1337, k.tr);
        TEST_I32X (0, k.sr & (SR_M | SR_U));
    }
}

