#include "ckone.h"


/**
 * Operation types for do_and_check ().
 */
typedef enum {
    ADD, SUB, MUL
} e_op;


/**
 * Perform an operation (add, subtract, multiply), check
 * whether the result overflows, and set the overflow bit
 * of SR if it does.
 */
void do_and_check (s_ckone* kone, e_op op) {
    int32_t a = kone->alu_in1;
    int32_t b = kone->alu_in2;

    int32_t result;
    int64_t real_result;

    switch (op) {
        case ADD: result = a + b; real_result = (int64_t)a + (int64_t)b; break;
        case SUB: result = a - b; real_result = (int64_t)a - (int64_t)b; break;
        case MUL: result = a * b; real_result = (int64_t)a * (int64_t)b; break;
    }

    if ((int64_t)result != real_result) {
        kone->sr |= SR_O;   // overflow
        char* opstr = " + ";
        if (op == SUB)
            opstr = " - ";
        else if (op == MUL)
            opstr = " * ";

        ELOG ("Integer overflow: %d%s%d\n", a, opstr, b);
    }

    kone->alu_out = result;
}


void alu_add (s_ckone* kone) {
    do_and_check (kone, ADD);
}


void alu_sub (s_ckone* kone) {
    do_and_check (kone, SUB);
} 


void alu_mul (s_ckone* kone) {
    do_and_check (kone, MUL);
}


void alu_div (s_ckone* kone) {
    if (kone->alu_in2 == 0) {
        kone->sr |= SR_Z;
        return;
    }

    kone->alu_out = kone->alu_in1 / kone->alu_in2;
}


void alu_mod (s_ckone* kone) {
    if (kone->alu_in2 == 0) {
        kone->sr |= SR_Z;
        return;
    }

    kone->alu_out = kone->alu_in1 % kone->alu_in2;
}


void alu_and (s_ckone* kone) {
    kone->alu_out = kone->alu_in1 & kone->alu_in2;
}


void alu_or (s_ckone* kone) {
    kone->alu_out = kone->alu_in1 | kone->alu_in2;
}


void alu_xor (s_ckone* kone) {
    kone->alu_out = kone->alu_in1 ^ kone->alu_in2;
}


void alu_not (s_ckone* kone) {
    kone->alu_out = ~kone->alu_in1;
}


void alu_shl (s_ckone* kone) {
    kone->alu_out = kone->alu_in1 << kone->alu_in2;
}


void alu_shr (s_ckone* kone) {
    kone->alu_out = kone->alu_in1 >> kone->alu_in2;
}


void alu_shra (s_ckone* kone) {
    int32_t msb = kone->alu_in1 & 0x80000000;
    int32_t res = kone->alu_in1;

    for (int i = 0; i < kone->alu_in2; i++) {
        res >>= 1;
        res |= msb;
    }

    kone->alu_out = res;
}

