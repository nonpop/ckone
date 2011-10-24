#include <string.h>
#include "../src/ckone.h"


/**
 * Clear the machine state and memory.
 *
 * The base and limits will be set to 0 and mem_size respectively.
 *
 * @param kone The state structure.
 */
void clear (s_ckone* kone) {
    uint32_t* mem = kone->mem;
    uint32_t mem_size = kone->mem_size;

    memset (mem, 0, mem_size*sizeof(uint32_t));
    memset (kone, 0, sizeof(s_ckone));
    kone->mem = mem;
    kone->mem_size = mem_size;
    kone->mmu_limit = mem_size;
}

