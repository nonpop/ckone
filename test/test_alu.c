#include <string.h>
#include "test.h"
#include "../src/alu.h"


void test_alu() {
    s_ckone k;
    memset (&k, 0, sizeof(s_ckone));

    BEGIN ("addition") {
        k.sr = 0;
        k.alu_in1 = 42;
        k.alu_in2 = 1337;
        alu_add (&k);
        TEST_I32 (42+1337, k.alu_out);
        TEST_BITSCLR (k.sr, SR_O);
    }
    BEGIN ("subtraction") {
        k.sr = 0;
        k.alu_in1 = 42;
        k.alu_in2 = -1337;
        alu_add (&k);
        TEST_I32 (42-1337, k.alu_out);
        TEST_BITSCLR (k.sr, SR_O);
    }
    BEGIN ("addition, overflow") {
        k.sr = 0;
        k.alu_in1 = 0x7fffffff;
        k.alu_in2 = 1;
        alu_add (&k);
        TEST_BITSSET (k.sr, SR_O);
    }
    BEGIN ("subtraction, overflow") {
        k.sr = 0;
        k.alu_in1 = 0;
        k.alu_in2 = 0x80000000;
        alu_sub (&k);
        TEST_BITSSET (k.sr, SR_O);
    }
    BEGIN ("division") {
        k.sr = 0;
        k.alu_in1 = 10;
        k.alu_in2 = 5;
        alu_div (&k);
        TEST_I32 (10/5, k.alu_out);
        TEST_BITSCLR (k.sr, SR_O | SR_Z);
    }
    BEGIN ("division by zero") {
        k.sr = 0;
        k.alu_in1 = 10;
        k.alu_in2 = 0;
        alu_div (&k);
        TEST_BITSSET (k.sr, SR_Z);
    }
    BEGIN ("shr positive") {
        k.sr = 0;
        k.alu_in1 = 0x00000002;
        k.alu_in2 = 1;
        alu_shr (&k);
        TEST_I32X (0x00000001, k.alu_out);
        k.alu_in1 = k.alu_out;
        alu_shr (&k);
        TEST_I32X (0x00000000, k.alu_out);
    }
    BEGIN ("shr negative") {
        k.sr = 0;
        k.alu_in1 = 0x80000002;
        k.alu_in2 = 1;
        alu_shr (&k);
        TEST_I32X (0x40000001, k.alu_out);
        k.alu_in1 = k.alu_out;
        alu_shr (&k);
        TEST_I32X (0x20000000, k.alu_out);
    }

    BEGIN ("shra positive") {
        k.sr = 0;
        k.alu_in1 = 0x00000002;
        k.alu_in2 = 1;
        alu_shra (&k);
        TEST_I32X (0x00000001, k.alu_out);
        k.alu_in1 = k.alu_out;
        alu_shra (&k);
        TEST_I32X (0x00000000, k.alu_out);
    }
    BEGIN ("shra negative") {
        k.sr = 0;
        k.alu_in1 = 0x80000002;
        k.alu_in2 = 1;
        alu_shra (&k);
        TEST_I32X (0xc0000001, k.alu_out);
        k.alu_in1 = k.alu_out;
        alu_shra (&k);
        TEST_I32X (0xe0000000, k.alu_out);
    }
}

