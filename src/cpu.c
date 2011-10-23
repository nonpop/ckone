#include "cpu.h"
#include "mmu.h"
#include "instr.h"


void cpu_fetch_instr (s_ckone* kone) {
    kone->mar = kone->pc++;
    mmu_read (kone);
    if (kone->sr & SR_M)    // check that the memory was successfully read
        return;

    kone->ir = kone->mbr;
}


uint32_t calculate_second_operand (s_ckone* kone) {
    uint32_t result = instr_addr (kone->ir);
    if (instr_index_reg (kone->ir) != R0)
        result += kone->r[instr_index_reg (kone->ir)];

    if (instr_addr_mode (kone->ir) > 2) {
        ELOG ("Invalid addressing mode", 0);
        kone->sr |= SR_U;
        return 0;
    }

    for (uint32_t i = 0; i < instr_addr_mode (kone->ir); i++) {
        kone->mar = result;
        mmu_read (kone);
        if (kone->sr & SR_M)
            return 0;

        result = kone->mbr;
    }

    DLOG ("The second operand is: 0x%x\n", result);
    return result;
}

