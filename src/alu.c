/**
 * @file alu.c
 *
 * Routines for the ALU operations.
 */

#include "common.h"


/**
 * @internal
 * Operation types for do_and_check().
 */
typedef enum {
    ADD, SUB, MUL
} e_op;


/**
 * @internal
 * Perform an operation (add, subtract, multiply), check
 * whether the result overflows, and set the overflow bit
 * of SR if it does.
 *
 * Affects: ALU_OUT
 *
 * Affected status bits: ::SR_O
 */
static void 
do_and_check (
        s_ckone* kone,  ///< The state structure.
        e_op op         ///< The operation to perform.
        ) 
{
    int32_t a = kone->alu_in1;
    int32_t b = kone->alu_in2;

    int32_t result = 0;
    int64_t real_result = 0;

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


/// @cond skip
// Macros to show some debug messages
#define MSG(op) DLOG ("Calculating 0x%x " op " 0x%x (%d " op " %d)\n", \
                      kone->alu_in1, kone->alu_in2, kone->alu_in1, kone->alu_in2);
#define RES() DLOG ("Result = 0x%x (%d)\n", kone->alu_out, kone->alu_out)
/// @endcond


/**
 * Perform an addition. ALU_IN1 and ALU_IN2 should contain the
 * operands and the result is stored in ALU_OUT.
 *
 * Affects: ALU_OUT
 *
 * Affected status bits: ::SR_O
 */
void 
alu_add (
        s_ckone* kone   ///< The state structure.
        ) 
{
    MSG ("+");
    do_and_check (kone, ADD);
    RES ();
}


/**
 * Perform a subtraction. ALU_IN1 and ALU_IN2 should contain the
 * operands and the result is stored in ALU_OUT.
 *
 * Affects: ALU_OUT
 *
 * Affected status bits: ::SR_O
 */
void 
alu_sub (
        s_ckone* kone   ///< The state structure.
        ) 
{
    MSG ("-");
    do_and_check (kone, SUB);
    RES ();
} 


/**
 * Perform a multiplication. ALU_IN1 and ALU_IN2 should contain the
 * operands and the result is stored in ALU_OUT.
 *
 * Affects: ALU_OUT
 *
 * Affected status bits: ::SR_O
 */
void 
alu_mul (
        s_ckone* kone   ///< The state structure.
        ) 
{
    MSG ("*");
    do_and_check (kone, MUL);
    RES ();
}


/**
 * Perform a division. ALU_IN1 and ALU_IN2 should contain the
 * operands and the whole part of the result is stored in ALU_OUT.
 *
 * Affects: ALU_OUT
 *
 * Affected status bits: ::SR_Z
 */
void 
alu_div (
        s_ckone* kone   ///< The state structure.
        ) 
{
    MSG ("/");
    if (kone->alu_in2 == 0) {
        kone->sr |= SR_Z;
        ELOG ("Division by zero.\n", 0);
        return;
    }

    kone->alu_out = kone->alu_in1 / kone->alu_in2;
    RES ();
}


/**
 * Perform a division. ALU_IN1 and ALU_IN2 should contain the
 * operands and the remainder part of the result is stored in ALU_OUT.
 *
 * Affects: ALU_OUT
 *
 * Affected status bits: ::SR_Z
 */
void 
alu_mod (
        s_ckone* kone   ///< The state structure.
        ) 
{
    MSG ("%%");
    if (kone->alu_in2 == 0) {
        kone->sr |= SR_Z;
        return;
    }

    kone->alu_out = kone->alu_in1 % kone->alu_in2;
    RES ();
}


/**
 * Perform a bitwise logical and. ALU_IN1 and ALU_IN2 should
 * contain the operands and the result is stored in ALU_OUT.
 *
 * Affects: ALU_OUT
 */
void 
alu_and (
        s_ckone* kone   ///< The state structure.
        ) 
{
    MSG ("&");
    kone->alu_out = kone->alu_in1 & kone->alu_in2;
    RES ();
}


/**
 * Perform a bitwise logical or. ALU_IN1 and ALU_IN2 should
 * contain the operands and the result is stored in ALU_OUT.
 *
 * Affects: ALU_OUT
 */
void 
alu_or (
        s_ckone* kone   ///< The state structure.
        ) 
{
    MSG ("|");
    kone->alu_out = kone->alu_in1 | kone->alu_in2;
    RES ();
}


/**
 * Perform a bitwise logical xor. ALU_IN1 and ALU_IN2 should
 * contain the operands and the result is stored in ALU_OUT.
 *
 * Affects: ALU_OUT
 */
void 
alu_xor (
        s_ckone* kone   ///< The state structure.
        ) 
{
    MSG ("^");
    kone->alu_out = kone->alu_in1 ^ kone->alu_in2;
    RES ();
}


/**
 * Perform a bitwise logical not. ALU_IN1 should contain the 
 * operand and the result is stored in ALU_OUT. The value in 
 * ALU_IN2 is ignored.
 *
 * Affects: ALU_OUT
 */
void 
alu_not (
        s_ckone* kone   ///< The state structure.
        ) 
{
    DLOG ("Calculating ~0x%x (~%d)\n", kone->alu_in1, kone->alu_in1);
    kone->alu_out = ~kone->alu_in1;
    RES ();
}


/**
 * Perform a bitwise left shift. ALU_IN1 and ALU_IN2 should
 * contain the operands and the result is stored in ALU_OUT.
 *
 * Affects: ALU_OUT
 */
void 
alu_shl (
        s_ckone* kone   ///< The state structure.
        ) 
{
    MSG ("SHL");
    kone->alu_out = kone->alu_in1 << kone->alu_in2;
    RES ();
}

/**
 * Perform a bitwise right shift. ALU_IN1 and ALU_IN2 should
 * contain the operands and the result is stored in ALU_OUT.
 *
 * Affects: ALU_OUT
 */
void 
alu_shr (
        s_ckone* kone   ///< The state structure.
        ) 
{
    MSG ("SHR");
    kone->alu_out = kone->alu_in1 >> kone->alu_in2;

    if (kone->alu_in2 > 0) {
        int32_t sign_bits = kone->alu_in1 & 0x80000000;
        sign_bits >>= (kone->alu_in2 - 1);
        kone->alu_out ^= sign_bits;
    }
    RES ();
}


/**
 * Perform a bitwise arithmetic right shift. ALU_IN1 and ALU_IN2
 * should contain the operands and the result is stored in ALU_OUT.
 *
 * Affects: ALU_OUT
 */
void 
alu_shra (
        s_ckone* kone   ///< The state structure.
        ) 
{
    MSG ("SHRA");
    kone->alu_out = kone->alu_in1 >> kone->alu_in2;
    RES ();
}

