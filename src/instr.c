/**
 * @file instr.c
 *
 * Instruction decoding/encoding functions.
 */

#include "common.h"
#include "instr.h"


/**
 * @internal 
 * Connects an opcode with its name.
 */
typedef struct {
    e_opcode opcode;        ///< The operation code.
    const char* name;       ///< The operation name.
} s_op_name;


/// @cond skip
// Make an opcode, opcode-name pair.
#define I(instr) { instr, #instr }
/// @endcond

/**
 * @internal
 * All opcodes with their names.
 */
static s_op_name op_names[] = {
    I(NOP),
    I(STORE), I(LOAD), I(IN), I(OUT),
    I(ADD), I(SUB), I(MUL), I(DIV), I(MOD),
    I(AND), I(OR), I(XOR), I(SHL), I(SHR), I(NOT), I(SHRA),
    I(COMP),
    I(JUMP), I(JNEG), I(JZER), I(JPOS), I(JNNEG), I(JNZER), I(JNPOS),
    I(JLES), I(JEQU), I(JGRE), I(JNLES), I(JNEQU), I(JNGRE),
    I(CALL), I(EXIT), I(PUSH), I(POP), I(PUSHR), I(POPR),
    I(SVC),
    { NOP, NULL }
};


/**
 * @internal
 * Return the name of the given operation.
 *
 * @return The name of the operation.
 */
static const char* 
op_name (
        e_opcode opcode     ///< The operation code.
        ) 
{
    s_op_name* on = op_names;
    while (on->name) {
        if (on->opcode == opcode)
            return on->name;

        on++;
    }

    return "(Unknown)";
}


/**
 * Extract the opcode of an instruction.
 *
 * @return The operation code of the instruction.
 */
e_opcode 
instr_opcode (
        int32_t instr       ///< The instruction.
        ) 
{
    return instr >> 24;
}


/**
 * Extract the first operand of an instruction.
 *
 * @return The first operand of the instruction.
 */
e_register 
instr_first_operand (
        int32_t instr       ///< The instruction.
        ) 
{
    return (instr >> 21) & 0x7;
}


/**
 * Extract the addressing mode of an instruction.
 *
 * @return The instruction addressing mode.
 */
e_addr_mode 
instr_addr_mode (
        int32_t instr       ///< The instruction.
        ) 
{
    return (instr >> 19) & 0x3;
}


/**
 * Extract the indexing register of an instruction.
 *
 * @return The indexing register of the instruction.
 */
e_register 
instr_index_reg (
        int32_t instr       ///< The instruction.
        ) 
{
    return (instr >> 16) & 0x7;
}


/**
 * Extract the address/constant part of an instruction.
 *
 * @return The constant part of the instruction.
 */
int16_t 
instr_addr (
        int32_t instr       ///< The instruction.
        ) 
{
    return (int16_t) instr;
}


/**
 * Assemble an instruction from its parts.
 *
 * @return The complete instruction.
 */
int32_t 
make_instr (
        e_opcode opcode,            ///< The opcode.
        e_register first_operand,   ///< The first operand.
        e_addr_mode addr_mode,      ///< The addressing mode.
        e_register index_reg,       ///< The indexing register.
        int16_t addr                ///< The address/constant part.
        )
{
    return (opcode << 24) 
         | (first_operand << 21) 
         | (addr_mode << 19) 
         | (index_reg << 16) 
         |  addr;
}


/**
 * @internal
 * Get the name of the given register.
 *
 * @return The name of the register (R0-R5, SP, FP).
 */
static char* 
reg_name (
        e_register r        ///< The register.
        ) 
{
    switch (r) {
        case R0: return "R0";
        case R1: return "R1";
        case R2: return "R2";
        case R3: return "R3";
        case R4: return "R4";
        case R5: return "R5";
        case SP: return "SP";
        case FP: return "FP";
    }
    return "This cannot happen";
}


/**
 * Get a textual representation of the given instruction.
 */
void 
instr_string (
        uint32_t instr,         ///< The instruction.
        char* buffer,           ///< The buffer to write the result to.
        size_t buf_size         ///< The size of the buffer.
        ) 
{
    snprintf (buffer, buf_size, "%s, first opr: %s, indirections: %u, "
            "index: %s, constant: 0x%04x (%d)",
            op_name (instr_opcode (instr)), reg_name (instr_first_operand (instr)),
            instr_addr_mode (instr), reg_name (instr_index_reg (instr)),
            instr_addr (instr), instr_addr (instr));
}

