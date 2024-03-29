#include "common.h"
#include "test.h"
#include "util.h"
#include "mmu.h"


void test_mmu () {
    s_ckone k;
    int32_t mem[2];

    k.mem_size = sizeof(mem)/sizeof(int32_t);
    k.mem = mem;
    
    {
        clear (&k);
        k.mmu_base = 1;
        k.mmu_limit = 1;
        k.mem[1] = 1337;
        k.mar = 0;

        mmu_read (&k);
        TEST (int32_t, "%u", 1337, k.mbr);
        TEST (int32_t, "0x%x", 0, k.sr & SR_M);
    }
    {
        clear (&k);
        k.mmu_base = 1;
        k.mmu_limit = 1;
        k.mar = 2;

        mmu_read (&k);
        TEST (int32_t, "0x%x", SR_M, k.sr & SR_M);
    }
    {
        clear (&k);
        k.mmu_base = 1;
        k.mmu_limit = 1;
        TEST (int32_t, "%u", 0, k.mem[1]);

        k.mar = 0;
        k.mbr = 42;

        mmu_write (&k);
        TEST (int32_t, "%u", 42, k.mem[1]);
    }
}

