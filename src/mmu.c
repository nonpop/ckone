/**
 * @file mmu.c
 *
 * Contains code which emulates the MMU.
 */

#include "common.h"


/**
 * @internal
 * Calculate the physical address of the given logical address.
 *
 * @return The physical (i.e. absolute) address.
 */
static uint32_t 
calculate_paddr (
        s_ckone* kone,      ///< The state structure.
        uint32_t laddr      ///< The logical address.
        ) 
{
    return kone->mmu_base + laddr;
}


/**
 * @internal
 * Check if the given physical address is within the
 * limits of MMU_BASE and MMU_LIMIT.
 *
 * @return true if the address can be accessed.
 */

static bool 
valid_paddr (
        s_ckone* kone,      ///< The state structure.
        int32_t paddr       ///< The physical address.
        ) 
{
    return paddr >= kone->mmu_base && paddr < kone->mmu_base + kone->mmu_limit;
}


/**
 * Read a word from memory.
 *
 * Calculates the physical address for MAR and reads data from
 * that memory address into MBR.
 *
 * Affects: MBR
 *
 * Affected status bits: ::SR_M
 */
void 
mmu_read (
        s_ckone* kone       ///< The state structure.
        ) 
{
    int32_t paddr = calculate_paddr (kone, kone->mar);

    if (!valid_paddr (kone, paddr)) {
        ELOG ("Tried to read from address 0x%x (%d) (base = 0x%x (%d), limit = 0x%x (%d))\n"
              "Try adding more memory using the --mem-size option, or adjusting the memory\n"
              "limit using the --mmu-limit option\n",
                paddr, paddr, kone->mmu_base, kone->mmu_base, kone->mmu_limit, kone->mmu_limit);

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
 * Affected status bits: ::SR_M
 */
void 
mmu_write (
        s_ckone* kone       ///< The state structure.
        ) 
{
    int32_t paddr = calculate_paddr (kone, kone->mar);

    if (!valid_paddr (kone, paddr)) {
        ELOG ("Tried to write to address 0x%x (%d) (base = 0x%x (%d), limit = 0x%x (%d))\n"
              "Try adding more memory using the --mem-size option, or adjusting the memory\n"
              "limit using the --mmu-limit option\n",
                paddr, paddr, kone->mmu_base, kone->mmu_base, kone->mmu_limit, kone->mmu_limit);

        kone->sr |= SR_M;
        return;
    }

    kone->mem[paddr] = kone->mbr;
    DLOG ("Wrote 0x%x to 0x%x\n", kone->mem[paddr], paddr);
}

