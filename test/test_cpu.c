#include <string.h>
#include "test.h"
#include "../src/cpu.h"
#include "../src/instr.h"


void test_cpu () {
    s_ckone k;
    uint32_t mem[2];
    memset (mem, 0, sizeof(mem));

    k.mem = mem;
    k.mem_size = sizeof(mem)/sizeof(uint32_t);
    k.mmu_base = 0;
    k.mmu_limit = k.mem_size;

    uint32_t instr = make_instr (LOAD, R3, DIRECT, R1, 42);
    mem[1] = instr;

    k.pc = 0;
    k.ir = 0;
    k.sr = 0;

    {
        TEST (uint32_t, "0x%x", 0, k.ir);
        cpu_fetch_instr (&k);
        TEST (uint32_t, "0x%x", 0, k.ir);
        TEST (uint32_t, "0x%x", 0, k.sr & SR_M);
        cpu_fetch_instr (&k);
        TEST (uint32_t, "0x%x", instr, k.ir);
        TEST (uint32_t, "0x%x", 0, k.sr & SR_M);
        cpu_fetch_instr (&k);
        TEST (uint32_t, "0x%x", SR_M, k.sr & SR_M);
    }
}

