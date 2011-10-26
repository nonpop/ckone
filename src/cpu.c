#include "cpu.h"
#include "mmu.h"
#include "instr.h"
#include "alu.h"
#include "ext.h"


/**
 * Fetch the next instruction to IR.
 *
 * Affects: MAR, MBR, PC, IR
 * Status: M (invalid memory access)
 */
void cpu_fetch_instr (s_ckone* kone) {
    kone->mar = kone->pc++;
    mmu_read (kone);
    kone->ir = kone->mbr;
}


/**
 * Calculates the second operand for the current instruction 
 * and stores it to the TR register.
 *
 * Affects: ALU_IN1, ALU_IN2, ALU_OUT, TR, MAR, MBR
 * Status: O (arithmetic overflow), M (invalid memory access),
 *         U (invalid addressing mode)
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


/**
 * Execute a store or load command.
 *
 * Affects: MAR, MBR, Rx
 * Status: M (invalid memory access)
 */
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
    if (instr_opcode (kone->ir) == IN) {
        ext_in (kone);
    } else {
        ext_out (kone);
    }
}


/**
 * Execute an arithmetic/logic command.
 *
 * Affects: ALU_IN1, ALU_IN2, ALU_OUT, Rx
 * Status: O (overflow), Z (division by zero
 */
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


/**
 * Execute a comp command.
 *
 * Status: L (less than), E (equal), G (greater)
 */
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


/**
 * Execute a jump command.
 *
 * Affects: PC
 */
void cpu_exec_jump (s_ckone* kone) {
    int32_t a = kone->r[instr_first_operand (kone->ir)];
    int32_t sr = kone->sr;
    bool jump = false;

    switch (instr_opcode (kone->ir)) {
        case JUMP: jump = true; break;
        case JNEG: if (a < 0) jump = true; break;
        case JZER: if (a == 0) jump = true; break;
        case JPOS: if (a > 0) jump = true; break;
        case JNNEG: if (a >= 0) jump = true; break;
        case JNZER: if (a != 0) jump = true; break;
        case JNPOS: if (a <= 0) jump = true; break;

        case JLES: if (sr & SR_L) jump = true; break;
        case JEQU: if (sr & SR_E) jump = true; break;
        case JGRE: if (sr & SR_G) jump = true; break;
        case JNLES: if (!(sr & SR_L)) jump = true; break;
        case JNEQU: if (!(sr & SR_E)) jump = true; break;
        case JNGRE: if (!(sr & SR_G)) jump = true; break;

        default: ELOG ("We should never get here", 0); break;
    }

    if (jump)
        kone->pc = kone->tr;
}


/**
 * Push PC and FP onto the stack and set FP = sp.
 *
 * Affects: MAR, MBR, Rx, FP
 * Status: M (invalid memory access)
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


/**
 * Pop FP and PC off the stack.
 *
 * Affects: MAR, MBR, Rx, FP
 * Status: M (invalid memory access)
 */
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


/**
 * Execute a call command.
 *
 * Affects: MAR, MBR, Rx, FP, PC
 * Status: M (invalid memory access)
 */
void cpu_exec_call (s_ckone* kone) {
    push_pc_fp (kone, instr_first_operand (kone->ir));
    kone->pc = kone->tr;
}


/**
 * Execute an exit command.
 *
 * Affects: MAR, MBR, Rx, FP, PC
 * Status: M (invalid memory access)
 */
void cpu_exec_exit (s_ckone* kone) {
    e_register sp = instr_first_operand (kone->ir);
    pop_fp_pc (kone, sp);
    kone->r[sp] -= kone->tr;    // remove parameters from stack
}


/**
 * Push a value onto the stack.
 *
 * @param sp The stack pointer.
 * @param value The value to push.
 *
 * Affects: MAR, MBR, Rsp
 * Status: M (invalid memory access)
 */
void cpu_push_value (s_ckone* kone, e_register sp, int32_t value) {
    kone->mbr = value;
    kone->r[sp]++;
    kone->mar = kone->r[sp];
    mmu_write (kone);
}


/**
 * Push a register onto the stack. It first stores the value in Rreg
 * onto the stack and then increases Rsp, so in case these are the same
 * register, the pushed value will be the original value.
 *
 * @param sp The stack pointer.
 * @param reg The register to push.
 *
 * Affects: MAR, MBR, Rsp
 * Status: M (invalid memory access)
 */
void cpu_push_register (s_ckone* kone, e_register sp, e_register reg) {
    cpu_push_value (kone, sp, kone->r[reg]);
}


/**
 * Pop a value off the stack and store it to a register.
 * It first assigns the value to Rreg and then decreases Rsp, so
 * in case these are the same register, the popped value will be
 * decreased by one.
 *
 * @param sp The stack pointer.
 * @param reg The register in which to store the value.
 *
 * Affects: MAR, MBR, Rsp, Rreg
 * Status: M (invalid memory access)
 */
void cpu_pop_register (s_ckone* kone, e_register sp, e_register reg) {
    kone->mar = kone->r[sp];
    mmu_read (kone);
    kone->r[reg] = kone->mbr;
    kone->r[sp]--;
}


/**
 * Execute a push command.
 *
 * @see cpu_push_value ()
 */
void cpu_exec_push (s_ckone* kone) {
    cpu_push_value (kone, instr_first_operand (kone->ir), kone->tr);
}


/**
 * Execute a pop command.
 *
 * @see cpu_pop_register ()
 */
void cpu_exec_pop (s_ckone* kone) {
    cpu_pop_register (kone, instr_first_operand (kone->ir), instr_index_reg (kone->ir));
}


/**
 * Execute a pushr command.
 *
 * @see cpu_push_register ()
 */
void cpu_exec_pushr (s_ckone* kone) {
    e_register sp = instr_first_operand (kone->ir);

    for (e_register i = R0; i <= R6; i++)
        cpu_push_register (kone, sp, i);
}


/**
 * Execute a popr command.
 *
 * @see cpu_pop_register ()
 */
void cpu_exec_popr (s_ckone* kone) {
    e_register sp = instr_first_operand (kone->ir);

    for (e_register i = R0; i <= R6; i--)
        cpu_pop_register (kone, sp, R6 - i);    // stupid unsigned integers ;p
}


/**
 * Execute a svc command.
 *
 * Affects: MAR, MBR, Rx, FP
 * Status: M (invalid memory access)
 */
void cpu_exec_svc (s_ckone* kone) {
    e_register sp = instr_first_operand (kone->ir);
    push_pc_fp (kone, sp);

    uint32_t params = ext_svc (kone);

    if (!kone->halted) {
        pop_fp_pc (kone, sp);
        kone->r[sp] -= params;
    }
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
    else if (op >= JUMP && op <= JNGRE)
        cpu_exec_jump (kone);
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

