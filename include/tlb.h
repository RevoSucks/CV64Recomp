#ifndef __TLB_H__
#define __TLB_H__

#ifdef __cplusplus
extern "C" {
#endif

#define OS_PM_4K	0x0000000
#define OS_PM_16K	0x0006000
#define OS_PM_64K	0x001E000
#define OS_PM_256K	0x007E000
#define OS_PM_1M	0x01FE000
#define OS_PM_4M	0x07FE000
#define OS_PM_16M	0x1FFE000

typedef uint32_t OSPageMask;

// TODO
struct TLBEntry {
    OSPageMask pm;
    uint32_t pagesize; // pre-computed for performance reasons
    uint32_t vaddr;
    uint32_t evenpaddr;
    uint32_t oddpaddr;
};

#define TLB_ENTRY_COUNT 32

extern struct TLBEntry gTLBTable[TLB_ENTRY_COUNT];

/**
 * RDRAM lookup for TLB.
 */
static inline int64_t _tlb_lookup(int64_t eff_addr) {
    // Fast path: is this normal RDRAM? — no TLB walk needed.
    if ((((uint64_t)eff_addr & 0x00000000FF000000) >> 24) == 0x80) {
        return eff_addr; // no need to process
    }

    uint32_t addr32 = (uint32_t)eff_addr;

    // Lookup the TLB table entry.
    for(int i = 0; i < TLB_ENTRY_COUNT; i++) {
        uint32_t pagesize = gTLBTable[i].pagesize;
        uint32_t fullsize = pagesize;
        int tlb_count = 0; // we need to keep track of the number of uses of addr field because effective page size matters

        if (gTLBTable[i].evenpaddr != -1) tlb_count++;
        if (gTLBTable[i].oddpaddr != -1) tlb_count++;

        if (tlb_count == 0) {
            continue; // skip empty entries.
        }

        // if both fields are used, the effective range is double due to 2 pages.
        if (tlb_count == 2) {
            fullsize += pagesize;
        }

        // is the address in the range?
        if (addr32 >= gTLBTable[i].vaddr && addr32 <= (gTLBTable[i].vaddr + fullsize)) {
            uint32_t offset = addr32 - gTLBTable[i].vaddr; // fetch the offset.
            int in_latter_mem = 0;
            uint32_t new_addr = 0;

            // if our offset is bigger than the pagesize, we need to use the later address.
            if (offset > pagesize) {
                in_latter_mem = 1;
                offset -= pagesize; // get the true offset. we need the bigger address.
                new_addr = (gTLBTable[i].oddpaddr > gTLBTable[i].evenpaddr) ? gTLBTable[i].oddpaddr : gTLBTable[i].evenpaddr;
            } else {
                // we need the lower address.
                new_addr = (gTLBTable[i].oddpaddr < gTLBTable[i].evenpaddr) ? gTLBTable[i].oddpaddr : gTLBTable[i].evenpaddr;
            }

            // now that we have the offset, add it to the base and 0x80000000 to get the physical RDRAM.
            new_addr += offset;
            new_addr += 0x80000000;

            return (int64_t)(int32_t)new_addr; // same here.
        }
    }
    printf("[_tlb_lookup] WARNING: Lookup failed. Defaulting to original address 0x%jX. Recomp may crash!\n", eff_addr);
    return eff_addr; // same here.
}

// TODO: Combine these.
/**
 * RDRAM lookup for TLB.
 */
static inline int64_t _tlb_lookup_raw(int64_t eff_addr) {
    // Fast path: is this normal RDRAM? — no TLB walk needed.
    if ((((uint64_t)eff_addr & 0x00000000FF000000) >> 24) == 0x80) {
        return eff_addr; // no need to process
    }

    uint32_t addr32 = (uint32_t)eff_addr;

    // Lookup the TLB table entry.
    for(int i = 0; i < TLB_ENTRY_COUNT; i++) {
        uint32_t pagesize = gTLBTable[i].pagesize;
        uint32_t fullsize = pagesize;
        int tlb_count = 0; // we need to keep track of the number of uses of addr field because effective page size matters

        if (gTLBTable[i].evenpaddr != -1) tlb_count++;
        if (gTLBTable[i].oddpaddr != -1) tlb_count++;

        if (tlb_count == 0) {
            continue; // skip empty entries.
        }

        // if both fields are used, the effective range is double due to 2 pages.
        if (tlb_count == 2) {
            fullsize += pagesize;
        }

        // is the address in the range?
        if (addr32 >= gTLBTable[i].vaddr && addr32 <= (gTLBTable[i].vaddr + fullsize)) {
            uint32_t offset = addr32 - gTLBTable[i].vaddr; // fetch the offset.
            int in_latter_mem = 0;
            uint32_t new_addr = 0;

            // if our offset is bigger than the pagesize, we need to use the later address.
            if (offset > pagesize) {
                in_latter_mem = 1;
                offset -= pagesize; // get the true offset. we need the bigger address.
                new_addr = (gTLBTable[i].oddpaddr > gTLBTable[i].evenpaddr) ? gTLBTable[i].oddpaddr : gTLBTable[i].evenpaddr;
            } else {
                // we need the lower address.
                new_addr = (gTLBTable[i].oddpaddr < gTLBTable[i].evenpaddr) ? gTLBTable[i].oddpaddr : gTLBTable[i].evenpaddr;
            }

            // now that we have the offset, add it to the base and 0x80000000 to get the physical RDRAM.
            new_addr += offset;

            return (int64_t)(int32_t)new_addr; // same here.
        }
    }
    printf("[_tlb_lookup] WARNING: Lookup failed. Defaulting to original address 0x%jX. Recomp may crash!\n", eff_addr);
    return eff_addr; // same here.
}

#ifdef __cplusplus
}
#endif

#endif // __TLB_H__
