#include "cpu.h"
#include "mmu.h"
#include "instr.h"
#include "alu.h"
#include "ext.h"


/**
 * Fetch the next instruction to IR.
 *
 * Affects: MAR, MBR, PC, IR
 * SR: M
 */
void cpu_fetch_instr (s_ckone* kone) {
    kone->mar = kone->pc++;
    mmu_read (kone);
    if (kone->sr & SR_M)    // check that the memory was successfully read
        return;

    kone->ir = kone->mbr;
}


/**
 * Calculates the second operand for the current instruction 
 * and stores it to the TR register.
 *
 * Affects: ALU_IN1, ALU_IN2, ALU_OUT, TR, MAR, MBR
 * SR: O, M, U
 */
void cpu_calculate_second_operand (s_ckone* kone) {
    // calculate the first address
    kone->alu_in1 = instr_addr (kone->ir);

    if (instr_index_reg (kone->ir) != R0)
        kone->alu_in2 = kone->r[instr_index_reg (kone->ir)];
    else
        kone->alu_in2 = 0;

    alu_add (kone);
    if (kone->sr & SR_O)
        return;

    kone->tr = kone->alu_out;
    
    
    // perform the memory fetches
    int mem_fetches = 0;
    switch (instr_addr_mode (kone->ir)) {
        case IMMEDIATE: mem_fetches = 0; break;
        case DIRECT: mem_fetches = 1; break;
        case INDIRECT: mem_fetches = 2; break;
        default:
            ELOG ("Invalid addressing mode", 0);
            kone->sr |= SR_U;
            return;
    }

    while (mem_fetches-- > 0) {
        kone->mar = kone->tr;
        mmu_read (kone);
        if (kone->sr & SR_M)
            return;

        kone->tr = kone->mbr;
    }

    DLOG ("The second operand is: 0x%x\n", kone->tr);
}


void cpu_exec_store_load (s_ckone* kone) {
    if (instr_opcode (kone->ir) == STORE) {
        kone->mar = kone->tr;
        kone->mbr = kone->r[instr_first_operand (kone->ir)];
        mmu_write (kone);
    } else {
        kone->r[instr_first_operand (kone->ir)] = kone->tr;
    }
}


void cpu_exec_in_out (s_ckone* kone) {
    e_register reg = instr_first_operand (kone->ir);

    if (instr_opcode (kone->ir) == IN) {
        int32_t res;
        if (!ext_input (kone->tr, &res)) {
            kone->sr |= SR_M;
            return;
        }
        kone->r[reg] = res;
    } else {
        if (!ext_output (kone->tr, kone->r[reg])) {
            kone->sr |= SR_M;
            return;
        }
    }
}


void cpu_exec_arithmetic (s_ckone* kone) {
    kone->alu_in1 = kone->r[instr_first_operand (kone->ir)];
    kone->alu_in2 = kone->tr;

    switch (instr_opcode (kone->ir)) {
        case ADD: alu_add (kone); break;
        case SUB: alu_sub (kone); break;
        case MUL: alu_mul (kone); break;
        case DIV: alu_div (kone); break;
        case MOD: alu_mod (kone); break;
        case AND: alu_and (kone); break;
        case OR: alu_or (kone); break;
        case XOR: alu_xor (kone); break;
        case SHL: alu_shl (kone); break;
        case SHR: alu_shr (kone); break;
        case NOT: alu_not (kone); break;
        case SHRA: alu_shra (kone); break;
        default: ELOG ("We should never get here", 0); break;
    }

    if (kone->sr & (SR_O | SR_Z))
        return;

    kone->r[instr_first_operand (kone->ir)] = kone->alu_out;
}


void cpu_exec_comp (s_ckone* kone) {
    kone->sr &= ~(SR_L | SR_E | SR_G);

    int32_t a = kone->r[instr_first_operand (kone->ir)];
    int32_t b = kone->tr;

    if (a < b)
        kone->sr |= SR_L;
    else if (a == b)
        kone->sr |= SR_E;
    else
        kone->sr |= SR_G;
}


void cpu_exec_jump (s_ckone* kone) {
    kone->pc = kone->tr;
}


void cpu_exec_jump_cond_register (s_ckone* kone) {
    int32_t a = kone->r[instr_first_operand (kone->ir)];
    bool jump = false;

    switch (instr_opcode (kone->ir)) {
        case JNEG: if (a < 0) jump = true; break;
        case JZER: if (a == 0) jump = true; break;
        case JPOS: if (a > 0) jump = true; break;
        case JNNEG: if (a >= 0) jump = true; break;
        case JNZER: if (a != 0) jump = true; break;
        case JNPOS: if (a <= 0) jump = true; break;
        default: ELOG ("We should never get here", 0); break;
    }

    if (jump)
        cpu_exec_jump (kone);
}


void cpu_exec_jump_cond_status (s_ckone* kone) {
    bool jump = false;
    int32_t sr = kone->sr;

    switch (instr_opcode (kone->ir)) {
        case JLES: if (sr & SR_L) jump = true; break;
        case JEQU: if (sr & SR_E) jump = true; break;
        case JGRE: if (sr & SR_G) jump = true; break;
        case JNLES: if (!(sr & SR_L)) jump = true; break;
        case JNEQU: if (!(sr & SR_E)) jump = true; break;
        case JNGRE: if (!(sr & SR_G)) jump = true; break;
        default: ELOG ("We should never get here", 0); break;
    }

    if (jump)
        cpu_exec_jump (kone);
}


/**
 * Push PC and FP onto the stack and set FP = sp.
 */
void push_pc_fp (s_ckone* kone, e_register sp) {
    kone->mar = kone->r[sp] + 1;
    kone->mbr = kone->pc;
    mmu_write (kone);
    kone->mar++;
    kone->mbr = kone->r[FP];
    mmu_write (kone);
    kone->r[sp] += 2;
    kone->r[FP] = kone->r[sp];
}


void pop_fp_pc (s_ckone* kone, e_register sp) {
    kone->mar = kone->r[sp];
    mmu_read (kone);
    int32_t fp = kone->mbr;
    kone->mar--;
    mmu_read (kone);
    kone->r[sp] -= 2;
    kone->r[FP] = fp;
    kone->pc = kone->mbr;
}


void cpu_exec_call (s_ckone* kone) {
    push_pc_fp (kone, instr_first_operand (kone->ir));
    if (kone->sr & SR_M)
        return;

    kone->pc = kone->tr;
}


void cpu_exec_exit (s_ckone* kone) {
    e_register sp = instr_first_operand (kone->ir);
    pop_fp_pc (kone, sp);
    if (kone->sr & SR_M)
        return;
    
    kone->r[sp] -= kone->tr;    // remove parameters from stack
}


void cpu_push_value (s_ckone* kone, e_register sp, int32_t value) {
    kone->mbr = value;
    kone->r[sp]++;
    kone->mar = kone->r[sp];
    mmu_write (kone);
}


void cpu_push_register (s_ckone* kone, e_register sp, e_register reg) {
    cpu_push_value (kone, sp, kone->r[reg]);
}


void cpu_pop_register (s_ckone* kone, e_register sp, e_register reg) {
    kone->mar = kone->r[sp];
    mmu_read (kone);
    if (kone->sr & SR_M)
        return;

    kone->r[reg] = kone->mbr;
    kone->r[sp]--;
}


void cpu_exec_push (s_ckone* kone) {
    cpu_push_value (kone, instr_first_operand (kone->ir), kone->tr);
}


void cpu_exec_pop (s_ckone* kone) {
    cpu_pop_register (kone, instr_first_operand (kone->ir), instr_index_reg (kone->ir));
}


void cpu_exec_pushr (s_ckone* kone) {
    e_register sp = instr_first_operand (kone->ir);

    for (e_register i = R0; i <= R6; i++) {
        cpu_push_register (kone, sp, i);
        if (kone->sr & SR_M)
            return;
    }
}


void cpu_exec_popr (s_ckone* kone) {
    e_register sp = instr_first_operand (kone->ir);

    for (e_register i = R0; i <= R6; i--) {
        cpu_pop_register (kone, sp, R6 - i);    // stupid unsigned integers ;p
        if (kone->sr & SR_M)
            return;
    }
}


void cpu_exec_svc (s_ckone* kone) {
    e_register sp = instr_first_operand (kone->ir);
    push_pc_fp (kone, sp);

    ext_svc (kone);

    if (!kone->halted)
        pop_fp_pc (kone, sp);
}


/**
 * Executes the current instruction. Assumes that the instruction has been
 * fetched and the second operand has been calculated and stored into TR.
 */
void cpu_execute_instruction (s_ckone* kone) {
    e_opcode op = instr_opcode (kone->ir);
    if (op == NOP)
        ;   // nothing
    else if (op == STORE || op == LOAD)
        cpu_exec_store_load (kone);
    else if (op == IN || op == OUT)
        cpu_exec_in_out (kone);
    else if (op >= ADD && op <= SHRA)
        cpu_exec_arithmetic (kone);
    else if (op == COMP)
        cpu_exec_comp (kone);
    else if (op == JUMP)
        cpu_exec_jump (kone);
    else if (op >= JNEG && op <= JNPOS)
        cpu_exec_jump_cond_register (kone);
    else if (op >= JLES && op <= JNGRE)
        cpu_exec_jump_cond_status (kone);
    else if (op == CALL)
        cpu_exec_call (kone);
    else if (op == EXIT)
        cpu_exec_exit (kone);
    else if (op == PUSH)
        cpu_exec_push (kone);
    else if (op == POP)
        cpu_exec_pop (kone);
    else if (op == PUSHR)
        cpu_exec_pushr (kone);
    else if (op == POPR)
        cpu_exec_popr (kone);
    else if (op == SVC)
        cpu_exec_svc (kone);
    else {
        ELOG ("Unknown opcode: %d\n", op);
        kone->sr |= SR_U;
    }
}


/**
 * Fetch the next instruction, calculate its second operand, and
 * execute it.
 */
void cpu_step (s_ckone* kone) {
    cpu_fetch_instr (kone);
    if (kone->sr & SR_M)
        return;

    cpu_calculate_second_operand (kone);
    if (kone->sr & (SR_O | SR_M | SR_U))
        return;
    
    char buf[1024];
    instr_string (kone->ir, buf, sizeof(buf));
    DLOG ("Executing %s\n", buf);
    cpu_execute_instruction (kone);
}

