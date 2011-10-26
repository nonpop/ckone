/**
 * @file
 *
 * @section DESCRIPTION
 * This contains functions which simulate the operations of the MMU.
 */


#include "mmu.h"


/**
 * Calculate the physical address of the given logical address.
 *
 * @param laddr The logical address (i.e. relative to mmu_base)
 * @return The physical address (i.e. absolute address)
 */
uint32_t calculate_paddr (s_ckone* kone, uint32_t laddr) {
    return kone->mmu_base + laddr;
}


/**
 * Check if the given physical address is valid.
 *
 * @param paddr The physical address.
 * @return true if the address can be accessed.
 */

bool valid_paddr (s_ckone* kone, int32_t paddr) {
    return paddr >= kone->mmu_base && paddr < kone->mmu_base + kone->mmu_limit;
}


/**
 * Read a word from memory.
 *
 * Calculates the physical address for MAR and reads data from
 * that memory address to MBR.
 *
 * Affects: MBR
 * Status: M (the physical address is outside the memory limits)
 */
void mmu_read (s_ckone* kone) {
    int32_t paddr = calculate_paddr (kone, kone->mar);

    if (!valid_paddr (kone, paddr)) {
        ELOG ("Tried to read from address 0x%x (base = 0x%x, limit = 0x%x)\n",
                paddr, kone->mmu_base, kone->mmu_limit);

        kone->sr |= SR_M;
        return;
    }

    kone->mbr = kone->mem[paddr];
    DLOG ("Read 0x%x from 0x%x\n", kone->mbr, paddr);
}


/**
 * Write a word to memory.
 *
 * Calculates the physical address for MAR and writes the contents of
 * MBR to that memory address.
 *
 * Status: M (the physical address is outside the memory limits)
 */
void mmu_write (s_ckone* kone) {
    int32_t paddr = calculate_paddr (kone, kone->mar);

    if (!valid_paddr (kone, paddr)) {
        ELOG ("Tried to write to address 0x%x (base = 0x%x, limit = 0x%x\n",
                paddr, kone->mmu_base, kone->mmu_limit);

        kone->sr |= SR_M;
        return;
    }

    kone->mem[paddr] = kone->mbr;
    DLOG ("Wrote 0x%x to 0x%x\n", kone->mem[paddr], paddr);
}

