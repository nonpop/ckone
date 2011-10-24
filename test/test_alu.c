#include <string.h>
#include "test.h"
#include "../src/alu.h"


void test_alu() {
    s_ckone k;
    memset (&k, 0, sizeof(s_ckone));

    {
        k.sr = 0;
        k.alu_in1 = 42;
        k.alu_in2 = 1337;
        alu_add (&k);
        TEST_I32 (42+1337, k.alu_out);
        TEST_BITSCLR (k.sr, SR_O);
    }
    {
        k.sr = 0;
        k.alu_in1 = 42;
        k.alu_in2 = -1337;
        alu_add (&k);
        TEST_I32 (42-1337, k.alu_out);
        TEST_BITSCLR (k.sr, SR_O);
    }
    {
        k.sr = 0;
        k.alu_in1 = 0x7fffffff;
        k.alu_in2 = 1;
        alu_add (&k);
        TEST_BITSSET (k.sr, SR_O);
    }
    {
        k.sr = 0;
        k.alu_in1 = 0;
        k.alu_in2 = 0x80000000;
        alu_sub (&k);
        TEST_BITSSET (k.sr, SR_O);
    }
    {
        k.sr = 0;
        k.alu_in1 = 10;
        k.alu_in2 = 5;
        alu_div (&k);
        TEST_I32 (10/5, k.alu_out);
        TEST_BITSCLR (k.sr, SR_O | SR_Z);
    }
    {
        k.sr = 0;
        k.alu_in1 = 10;
        k.alu_in2 = 0;
        alu_div (&k);
        TEST_BITSSET (k.sr, SR_Z);
    }
}

