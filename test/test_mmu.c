#include <string.h>
#include <stdlib.h>
#include "test.h"
#include "../src/mmu.h"


void clear (s_ckone* kone) {
    uint32_t* mem = kone->mem;
    uint32_t mem_size = kone->mem_size;

    memset (kone, 0, sizeof(s_ckone));
    kone->mem = mem;
    kone->mem_size = mem_size;

    memset (kone->mem, 0, kone->mem_size*sizeof(uint32_t));

    kone->mmu_base = 1;
    kone->mmu_limit = 2;
}


void test_mmu () {
    s_ckone k;
    const uint32_t mem_size = 2;
    k.mem_size = mem_size;
    k.mem = malloc (mem_size * sizeof(uint32_t));
    
    {
        clear (&k);
        k.mem[1] = 1337;
        k.mar = 0;

        mmu_read (&k);
        TEST (uint32_t, "%u", 1337, k.mbr);
        TEST (uint32_t, "0x%x", 0, k.sr & SR_M);
    }
    {
        clear (&k);
        k.mar = 2;

        mmu_read (&k);
        TEST (uint32_t, "0x%x", SR_M, k.sr & SR_M);
    }
    {
        clear (&k);
        TEST (uint32_t, "%u", 0, k.mem[1]);

        k.mar = 0;
        k.mbr = 42;

        mmu_write (&k);
        TEST (uint32_t, "%u", 42, k.mem[1]);
    }

    free (k.mem);
}

