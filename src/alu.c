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
static void do_and_check (s_ckone* kone, e_op op) {
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


// Macros for debugging output
#define MSG(op) DLOG ("Calculating 0x%x " op " 0x%x (%d + %d)\n", \
        kone->alu_in1, kone->alu_in2, kone->alu_in1, kone->alu_in2);
#define RES() DLOG ("Result = 0x%x (%d)\n", kone->alu_out, kone->alu_out)

void alu_add (s_ckone* kone) {
    MSG ("+");
    do_and_check (kone, ADD);
    RES ();
}


void alu_sub (s_ckone* kone) {
    MSG ("-");
    do_and_check (kone, SUB);
    RES ();
} 


void alu_mul (s_ckone* kone) {
    MSG ("*");
    do_and_check (kone, MUL);
    RES ();
}


void alu_div (s_ckone* kone) {
    MSG ("/");
    if (kone->alu_in2 == 0) {
        kone->sr |= SR_Z;
        return;
    }

    kone->alu_out = kone->alu_in1 / kone->alu_in2;
    RES ();
}


void alu_mod (s_ckone* kone) {
    MSG ("%%");
    if (kone->alu_in2 == 0) {
        kone->sr |= SR_Z;
        return;
    }

    kone->alu_out = kone->alu_in1 % kone->alu_in2;
    RES ();
}


void alu_and (s_ckone* kone) {
    MSG ("&");
    kone->alu_out = kone->alu_in1 & kone->alu_in2;
    RES ();
}


void alu_or (s_ckone* kone) {
    MSG ("|");
    kone->alu_out = kone->alu_in1 | kone->alu_in2;
    RES ();
}


void alu_xor (s_ckone* kone) {
    MSG ("^");
    kone->alu_out = kone->alu_in1 ^ kone->alu_in2;
    RES ();
}


void alu_not (s_ckone* kone) {
    DLOG ("Calculating ~0x%x (~%d)\n", kone->alu_in1, kone->alu_in1);
    kone->alu_out = ~kone->alu_in1;
    RES ();
}


void alu_shl (s_ckone* kone) {
    MSG ("SHL");
    kone->alu_out = kone->alu_in1 << kone->alu_in2;
    RES ();
}

void alu_shr (s_ckone* kone) {
    MSG ("SHR");
    kone->alu_out = kone->alu_in1 >> kone->alu_in2;

    if (kone->alu_in2 > 0) {
        int32_t sign_bits = kone->alu_in1 & 0x80000000;
        sign_bits >>= (kone->alu_in2 - 1);
        kone->alu_out ^= sign_bits;
    }
    RES ();
}


void alu_shra (s_ckone* kone) {
    MSG ("SHRA");
    kone->alu_out = kone->alu_in1 >> kone->alu_in2;
    RES ();
}

