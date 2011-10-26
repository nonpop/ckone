#include "test.h"
#include "util.h"
#include "../src/cpu.h"
#include "../src/instr.h"


void test_ext () {
    s_ckone k;
    int32_t mem[16];
    
    k.mem = mem;
    k.mem_size = sizeof(mem)/sizeof(int32_t);

    BEGIN ("in/out") {
        clear (&k);

        mem[0] = 52428801;      // in r1, =kbd
        mem[1] = 54525953;      // in r2, =kbd
        mem[2] = 287440896;     // add r1, r2
        mem[3] = 69206016;      // out r1, =crt

        k.input = 42;
        cpu_step (&k);          // in r1, =kbd
        TEST_I32 (42, k.r[R1]);
        TEST_I32 (0, k.sr);
        k.input = 1337;
        cpu_step (&k);          // in r2, =kbd
        TEST_I32 (1337, k.r[R2]);
        TEST_I32 (0, k.sr);
        cpu_step (&k);          // add r1, r2
        cpu_step (&k);          // out r1, =crt
        TEST_I32 (42+1337, k.output);
        TEST_I32 (0, k.sr);
    }
}

