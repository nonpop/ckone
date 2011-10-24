#include <string.h>
#include <stdlib.h>
#include "test.h"
#include "util.h"
#include "../src/mmu.h"


void test_mmu () {
    s_ckone k;
    uint32_t mem[2];

    k.mem_size = sizeof(mem);
    k.mem = mem;
    
    {
        clear (&k);
        k.mmu_base = 1;
        k.mmu_limit = 2;
        k.mem[1] = 1337;
        k.mar = 0;

        mmu_read (&k);
        TEST (uint32_t, "%u", 1337, k.mbr);
        TEST (uint32_t, "0x%x", 0, k.sr & SR_M);
    }
    {
        clear (&k);
        k.mmu_base = 1;
        k.mmu_limit = 2;
        k.mar = 2;

        mmu_read (&k);
        TEST (uint32_t, "0x%x", SR_M, k.sr & SR_M);
    }
    {
        clear (&k);
        k.mmu_base = 1;
        k.mmu_limit = 2;
        TEST (uint32_t, "%u", 0, k.mem[1]);

        k.mar = 0;
        k.mbr = 42;

        mmu_write (&k);
        TEST (uint32_t, "%u", 42, k.mem[1]);
    }
}

