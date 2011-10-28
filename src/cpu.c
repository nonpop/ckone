/**
 * @file cpu.c
 *
 * The main part of the CPU. Contains code for all operations except the 
 * arithmetic/logic operations and operations involving the external world 
 * (IN, OUT, SVC). Also contains code for performing one execution cycle. 
 *
 * Calls functions in alu.c and ext.c to perform operations not implemented
 * here, functions in mmu.c to read and write memory, and functions in
 * instr.c to decode instructions.
 */

#include "common.h"
#include "instr.h"
#include "alu.h"
#include "mmu.h"
#include "ext.h"
#include "args.h"


/**
 * @internal
 * Fetch the next instruction to IR.
 *
 * Affects: MAR, MBR, PC, IR
 *
 * Affected status bits: ::SR_M
 */
static void 
cpu_fetch_instr (
        s_ckone* kone       ///< The state structure.
        ) 
{
    DLOG ("Fetching instruction...\n", 0);
    kone->mar = kone->pc++;
    mmu_read (kone);
    kone->ir = kone->mbr;
}


/**
 * @internal
 * Calculates the second operand for the current instruction 
 * and stores it to the TR register.
 *
 * Affects: ALU_IN1, ALU_IN2, ALU_OUT, TR, MAR, MBR
 *
 * Affected status bits: ::SR_O, ::SR_M, ::SR_U
 */
static void 
cpu_calculate_second_operand (
        s_ckone* kone       ///< The state structure.
        ) 
{
    DLOG ("Calculating second operand...\n", 0);
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

    DLOG ("Second operand 1/%d: 0x%x (%d)\n", mem_fetches+1, kone->tr, kone->tr);
    
    for (int i = 0; i < mem_fetches; i++) {
        kone->mar = kone->tr;
        mmu_read (kone);
        if (kone->sr & SR_M)
            return;

        kone->tr = kone->mbr;
        DLOG ("Second operand %d/%d: 0x%x (%d)\n", 
                i+2, mem_fetches+1, kone->tr, kone->tr);
    }
}


/**
 * @internal
 * Execute a STORE or LOAD command.
 *
 * Affects: 
 *  - MAR, MBR (STORE)
 *  - Rx (LOAD)
 *
 * Affected status bits: ::SR_M
 */
static void 
cpu_exec_store_load (
        s_ckone* kone       ///< The state structure.
        ) 
{
    if (instr_opcode (kone->ir) == STORE) {
        kone->mar = kone->tr;
        kone->mbr = kone->r[instr_first_operand (kone->ir)];
        mmu_write (kone);
    } else {
        kone->r[instr_first_operand (kone->ir)] = kone->tr;
    }
}


/**
 * @internal
 * Execute an IN or OUT command.
 * See ext_in(), ext_out().
 *
 * Affects: Rx (IN)
 *
 * Affected status bits: ::SR_M
 */
static void 
cpu_exec_in_out (
        s_ckone* kone       ///< The state structure.
        ) 
{
    if (instr_opcode (kone->ir) == IN)
        ext_in (kone);
    else
        ext_out (kone);
}


/**
 * @internal
 * Execute an arithmetic/logic command.
 *
 * Affects: ALU_IN1, ALU_IN2, ALU_OUT, Rx
 *
 * Affected status bits: 
 *  - ::SR_O (ADD, SUB, MUL)
 *  - ::SR_Z (DIV, MOD)
 */
static void 
cpu_exec_arithmetic (
        s_ckone* kone       ///< The state structure.
        ) 
{
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
 * @internal
 * Execute a COMP command.
 *
 * Affected status bits: ::SR_L, ::SR_E, ::SR_G
 */
static void 
cpu_exec_comp (
        s_ckone* kone       ///< The state structure.
        ) 
{
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
 * @internal
 * Execute a jump command.
 *
 * Affects: PC
 */
static void 
cpu_exec_jump (
        s_ckone* kone       ///< The state structure.
        ) 
{
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
 * @internal
 * Push PC and FP onto the stack and set FP = sp.
 *
 * Affects: MAR, MBR, Rx, FP
 *
 * Affected status bits: ::SR_M
 */
static void 
push_pc_fp (
        s_ckone* kone,          ///< The state structure.
        e_register sp           ///< The stack pointer.
        ) 
{
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
 * @internal
 * Pop FP and PC off the stack.
 *
 * Affects: MAR, MBR, Rx, FP
 * 
 * Affected status bits: ::SR_M
 */
static void 
pop_fp_pc (
        s_ckone* kone,          ///< The state structure.
        e_register sp           ///< The stack pointer.
        ) 
{
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
 * @internal
 * Execute a CALL command.
 *
 * Affects: MAR, MBR, Rx, FP, PC
 *
 * Affected status bits: ::SR_M
 */
static void 
cpu_exec_call (
        s_ckone* kone       ///< The state structure.
        ) 
{
    push_pc_fp (kone, instr_first_operand (kone->ir));
    kone->pc = kone->tr;
}


/**
 * @internal
 * Execute an EXIT command.
 *
 * Affects: MAR, MBR, Rx, FP, PC
 *
 * Affected status bits: ::SR_M
 */
static void 
cpu_exec_exit (
        s_ckone* kone       ///< The state structure.
        ) 
{
    e_register sp = instr_first_operand (kone->ir);
    pop_fp_pc (kone, sp);
    kone->r[sp] -= kone->tr;    // remove parameters from stack
}


/**
 * @internal
 * Execute a PUSH command.
 *
 * Increases the value of the first operand register and then
 * stores the value in TR (the second operand) into memory at
 * the location pointed by the first operand.
 *
 * Affects: MAR, MBR, Rx
 *
 * Affected status bits: ::SR_M
 */
static void 
cpu_exec_push (
        s_ckone* kone       ///< The state structure.
        ) 
{
    e_register sp = instr_first_operand (kone->ir);
    kone->r[sp]++;
    kone->mar = kone->r[sp];
    kone->mbr = kone->tr;
    mmu_write (kone);
}


/**
 * @internal
 * Execute a POP command.
 *
 * First stores the value pointed by the first operand to the index
 * register of the instruction, then decreases the value of the
 * first operand register. @note If both registers are the same, the
 * popped value will be decreased by one.
 *
 * Affects: MAR, MBR, Rx, Ri
 *
 * Affected status bits: ::SR_M
 */
static void 
cpu_exec_pop (
        s_ckone* kone       ///< The state structure.
        ) 
{
    e_register sp = instr_first_operand (kone->ir);
    kone->mar = kone->r[sp];
    mmu_read (kone);
    kone->r[instr_index_reg (kone->ir)] = kone->mbr;
    kone->r[sp]--;
}


/**
 * @internal
 * Execute a PUSHR command.
 *
 * For each register R0 to R6, it first increases the first operand
 * register's value by one, then takes the value of one of the registers
 * and stores it to the memory location pointed by the first operand.
 * This means that the <i>increased</i> value of the register used as the 
 * stack pointer will be stored, contrary to how PUSH works.
 */
static void 
cpu_exec_pushr (
        s_ckone* kone       ///< The state structure.
        ) 
{
    e_register sp = instr_first_operand (kone->ir);

    for (e_register r = R0; r <= R6; r++) {
        kone->r[sp]++;
        kone->mar = kone->r[sp];
        kone->mbr = kone->r[r];
        mmu_write (kone);
    }
}


/**
 * @internal
 * Execute a POPR command.
 *
 * For each register R6 to R0, it first stores the value at the location
 * pointed by the first operand to one of the registers, then decreases
 * the value of the first operand register. This works the same way as
 * a normal POP.
 *
 * Affects: MAR, MBR, Rx, Ri
 *
 * Affected status bits: ::SR_M
 */
static void 
cpu_exec_popr (
        s_ckone* kone       ///< The state structure.
        ) 
{
    e_register sp = instr_first_operand (kone->ir);

    for (e_register i = R0; i <= R6; i++) {
        e_register r = R6 - i;                  // damn unsigned integers :p
        kone->mar = kone->r[sp];
        mmu_read (kone);
        kone->r[r] = kone->mbr;
        kone->r[sp]--;
    }
}


/**
 * @internal
 * Execute a SVC command.
 * See ext_svc().
 *
 * Affects: MAR, MBR, Rx, FP
 *
 * Affected status bits: ::SR_M
 */
static void 
cpu_exec_svc (
        s_ckone* kone       ///< The state structure.
        ) 
{
    e_register sp = instr_first_operand (kone->ir);
    push_pc_fp (kone, sp);
    DLOG ("FP is now 0x%x\n", kone->r[FP]);

    uint32_t params = ext_svc (kone);

    if (!kone->halted) {
        pop_fp_pc (kone, sp);
        kone->r[sp] -= params;
    }
}


/**
 * @internal
 * Execute the current instruction. Assumes that the instruction has been
 * fetched and the second operand has been calculated and stored into TR.
 */
static void 
cpu_execute_instruction (
        s_ckone* kone       ///< The state structure.
        ) 
{
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
 * Perform one execution cycle. Fetch the next instruction, 
 * calculate its second operand, and execute it.
 *
 * @return True if everything succeeded.
 */
bool 
cpu_step (
        s_ckone* kone       ///< The state structure.
        ) 
{
    cpu_fetch_instr (kone);
    if (kone->sr & SR_M)
        return false;

    char buf[1024];
    instr_string (kone->ir, buf, sizeof(buf));
    ILOG ("Executing %s\n", buf);

    cpu_calculate_second_operand (kone);
    if (kone->sr & (SR_O | SR_M | SR_U))
        return false;
    
    cpu_execute_instruction (kone);
    if (kone->sr & ~(SR_L | SR_E | SR_G))
        return false;

    if (args.step)
        ILOG ("Instruction finished.\n", 0);
    else
        DLOG ("Instruction finished.\n", 0);

    return true;
}

