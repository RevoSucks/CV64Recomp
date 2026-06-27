#include "recomp.h"
#include "librecomp/overlays.hpp"
#include "librecomp/helpers.hpp"
#include "ultramodern/ultramodern.hpp"
#include "tlb.h"

extern "C" {
struct TLBEntry gTLBTable[TLB_ENTRY_COUNT];
}

#define TLB_DEBUG

extern "C" void osMapTLB_recomp(uint8_t* rdram, recomp_context* ctx) {
    s32 index = _arg<0, s32>(rdram, ctx);
    OSPageMask pm = _arg<1, OSPageMask>(rdram, ctx);
    PTR(void) vaddr = _arg<2, PTR(void)>(rdram, ctx);
    u32 evenpaddr = _arg<3, u32>(rdram, ctx);
    u32 oddpaddr = (u32)MEM_W(0x10, ctx->r29);
    u32 asid = (u32)MEM_W(0x14, ctx->r29);

    u32 pagesize = 0;

#ifdef TLB_DEBUG
    printf("[osMapTLB] Mapping osMapTLB index %d with arguments: 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X\n", index, pm, vaddr, evenpaddr, oddpaddr, asid);
#endif

    if (asid != -1) {
        printf("[osMapTLB] WARNING: asid is not supported yet. Unpredictable TLB behavior may occur.\n");
    }

    switch(pm) {
        case OS_PM_4K:   pagesize = 4096;     break; // 0x1000
        case OS_PM_16K:  pagesize = 16384;    break; // 0x4000
        case OS_PM_64K:  pagesize = 65536;    break; // 0x10000
        case OS_PM_256K: pagesize = 262144;   break; // 0x40000
        case OS_PM_1M:   pagesize = 1048576;  break; // 0x100000
        case OS_PM_4M:   pagesize = 4194304;  break; // 0x400000
        case OS_PM_16M:  pagesize = 16777216; break; // 0x1000000
        default:
            printf("[osMapTLB] WARNING: unknown page mask 0x%08X. Unpredictable TLB behavior may occur.\n", pm);
            pagesize = 4096; // default to 4K, though this may still crash
            break;
    }

    gTLBTable[index].pm = pm;
    gTLBTable[index].pagesize = pagesize;
    gTLBTable[index].vaddr = vaddr;
    gTLBTable[index].evenpaddr = evenpaddr;
    gTLBTable[index].oddpaddr = oddpaddr;
}

extern "C" void osUnmapTLB_recomp(uint8_t* rdram, recomp_context* ctx) {
    s32 index = _arg<0, s32>(rdram, ctx);

    gTLBTable[index].pm = 0;
    gTLBTable[index].pagesize = 0;
    gTLBTable[index].vaddr = 0;
    gTLBTable[index].evenpaddr = 0;
    gTLBTable[index].oddpaddr = 0;
}

extern "C" void osUnmapTLBAll_recomp(uint8_t* rdram, recomp_context* ctx) {
    for (int i = 0; i < TLB_ENTRY_COUNT; i++) {
        gTLBTable[i].pm = 0;
        gTLBTable[i].pagesize = 0;
        gTLBTable[i].vaddr = 0;
        gTLBTable[i].evenpaddr = 0;
        gTLBTable[i].oddpaddr = 0;
    }
}
