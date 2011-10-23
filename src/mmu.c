/**
 * @file
 *
 * @section DESCRIPTION
 * This contains functions which simulate the operations of the MMU.
 */


#include "mmu.h"


/**
 * Calculate the effective address of the given address.
 *
 * @param kone The state structure.
 * @param addr The address for which to calculate the effective address.
 * @return The effective address.
 */
uint32_t calculate_eaddr (s_ckone* kone, uint32_t addr) {
    return kone->mmu_base + addr;
}


/**
 * Check if the given effective address is valid.
 *
 * @param kone The state structure.
 * @param eaddr The effective address.
 * @return true if the address can be accessed.
 */

bool valid_eaddr (s_ckone* kone, uint32_t eaddr) {
    return eaddr >= kone->mmu_base && eaddr < kone->mmu_base + kone->mmu_limit;
}


/**
 * Read a word from memory.
 *
 * Calculates the effective address for MAR and reads data from
 * that memory address to MBR. Sets the SR_M bit of SR if the address
 * is outside the limits.
 *
 * @param kone The state structure.
 */
void mmu_read (s_ckone* kone) {
    uint32_t eaddr = calculate_eaddr (kone, kone->mar);

    if (!valid_eaddr (kone, eaddr)) {
        ELOG ("Tried to read from address 0x%x (base = 0x%x, limit = 0x%x)\n",
                eaddr, kone->mmu_base, kone->mmu_limit);

        kone->sr |= SR_M;
        return;
    }

    kone->mbr = kone->mem[eaddr];
    DLOG ("Read 0x%x from 0x%x\n", kone->mbr, eaddr);
}


/**
 * Write a word to memory.
 *
 * Calculates the effective address for MAR and writes the contents of
 * MBR to that memory address. Sets the SR_M bit of SR if the address
 * is outside the limits.
 *
 * @param kone The state structure.
 */
void mmu_write (s_ckone* kone) {
    uint32_t eaddr = calculate_eaddr (kone, kone->mar);

    if (!valid_eaddr (kone, eaddr)) {
        ELOG ("Tried to write to address 0x%x (base = 0x%x, limit = 0x%x\n",
                eaddr, kone->mmu_base, kone->mmu_limit);

        kone->sr |= SR_M;
        return;
    }

    kone->mem[eaddr] = kone->mbr;
    DLOG ("Wrote 0x%x to 0x%x\n", kone->mem[eaddr], eaddr);
}

