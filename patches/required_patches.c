#include "patches.h"
#include "misc_funcs.h"
#include "PR/os_pi.h"
#include "game/system_work.h"
#include "game/object_ID.h"

unsigned long long dummy = 0x0123456789ABCDEFULL;

// osVirtualToPhysical patch because N64Recomp

RECOMP_PATCH u32 osVirtualToPhysical_game(void *addr) {
    u32 addr_val = (u32)addr;
    if (IS_KSEG0(addr_val)) {
        return K0_TO_PHYS(addr_val);
    } else if (IS_KSEG1(addr_val)) {
        return K1_TO_PHYS(addr_val);
    } else {
        // TODO handle TLB mappings
        return recomp_tlb_lookup(addr_val);
    }
}

struct FileSegment {
    u32 start;
    u32 end;
};

extern u32 D_8009502C_95C2C[];
extern struct FileSegment D_80094838_95438[];

#define FILE_NONE 0
#define FILE_MAX  0xFFu
#define FILE_COMP_MASK 0x80000000
#define FILE_ADDR_MASK 0x7FFFFFFF

extern OSPiHandle* gCartHandle;
extern void DMA_readWrite(OSPiHandle* piHandle, s32 direction, void* dest, void* src, s32 len);
extern void overlay_apply_relocations(u32 file_id, u8 *load_addr);

extern void romCopyAndDecompress(u32, void *, u32);

RECOMP_PATCH void DMA_ROMCopy(void* src, void* dest, s32 len) {
    recomp_printf("[DMA_ROMCopy] src 0x%08X src 0x%08X src 0x%08X\n", (u32)src, (u32)dest, len);

    switch((u32)src) {
        case 0x000A8420: 
        case 0x00697040: 
        case 0x006A42D0: 
        case 0x006B12F0: 
        case 0x006B4FC0: 
        case 0x006B5480: 
        case 0x006BAB80: 
        case 0x006BFBF0: 
        case 0x006C1460: 
        case 0x006C2F20: 
        case 0x006C3300: 
        case 0x006C50E0: 
        case 0x006C5110: 
        case 0x006C77D0: 
        case 0x006C86A0: 
        case 0x006C9BF0: 
        case 0x006CB850: 
        case 0x006CD050: 
        case 0x006CD740: 
        case 0x006CE4B0: 
        case 0x006CE9A0: 
        case 0x006CF420: 
        case 0x006D35A0: 
        case 0x006D74F0: 
        case 0x006E31D0: 
        case 0x006E86B0: 
        case 0x006EA480: 
        case 0x006EBD90: 
        case 0x006F0150: 
        case 0x006F0A50: 
        case 0x006F11B0: 
        case 0x006F1B80: 
        case 0x006F21D0: 
            recomp_load_overlays((u32)src, dest, len); break;
    }

    DMA_readWrite(gCartHandle, OS_READ, dest, src, len);
}

extern u32 currently_mapped_overlay_evenpaddr[];
extern u32 currently_mapped_overlay_size[];
extern u32 currently_mapped_overlay_vaddr[];
extern int mapped_files_array_index;
extern u32 D_80387DB8[];

struct UnkStruct80090000 {
    char filler0[0x342C];
    u32 unk342C;
};

extern u32 D_8009342C_9402C[];

struct UnkStruct80094838 {
    u32 unk0[1];
};

extern u32 D_80092270_92E70[];

struct UnkStruct_DMAMgr_loadFile_TempV0 {
    u32 unk0;
    u32 unk4;
    u32 unk8;
};

struct UnkStruct_DMAMgr_loadFile_TempA3 {
    char filler0[0x1];
};

extern u32 compressed_file__ID_from_ROMFileTable;

extern u32 D_80387C74; // unk

extern void NisitenmaIchigo_storeLoadedFile(void *, int);
extern void DMAMgr_8000f5d8(s32, s32, s32, s32, s32);

// Map overlay

extern u32 currently_mapped_overlay_evenpaddr[];
extern u32 currently_mapped_overlay_size[];
extern u32 currently_mapped_overlay_vaddr[];
extern int mapped_files_array_index;
extern u32 D_80387DB8[];

extern u32 D_80092270_92E70[];

u32 get_decompressed_rom(u32 rom) {
    recomp_printf("[get_decompressed_rom] input 0x%08X\n", rom);

    switch(rom) {
        case 0xA1560C: return 0xC95BA0; // ni_ovl_A1560C
        case 0xA20348: return 0xCA39F0; // ni_ovl_A20348
        case 0xA253B6: return 0xCAD050; // ni_ovl_A253B6
        case 0xA2945C: return 0xCB2650; // ni_ovl_A2945C
        case 0xA2DE52: return 0xCB8C00; // ni_ovl_A2DE52
        case 0xA31B06: return 0xCC00B0; // ni_ovl_A31B06
        case 0xA41292: return 0xCD6550; // ni_ovl_A41292
        case 0xA45FFA: return 0xCDCB40; // ni_ovl_A45FFA
        case 0xA4C58C: return 0xCE4C30; // ni_ovl_A4C58C
        case 0xA52788: return 0xCECE90; // ni_ovl_A52788
        case 0xA54ABA: return 0xCEFC10; // ni_ovl_A54ABA
        case 0xA5914A: return 0xCF5ED0; // ni_ovl_A5914A
        case 0xA591D6: return 0xCF6040; // ni_ovl_A591D6
        case 0xA59542: return 0xCF64F0; // ni_ovl_A59542
        case 0xA5B666: return 0xCF8F50; // ni_ovl_A5B666
        case 0xA5E06C: return 0xCFC360; // ni_ovl_A5E06C
        case 0xA5F57E: return 0xCFDD90; // ni_ovl_A5F57E
        case 0xA62486: return 0xD01E00; // ni_ovl_A62486
        case 0xA7A708: return 0xD235F0; // ni_ovl_A7A708
        case 0xA7C0FE: return 0xD25CA0; // ni_ovl_A7C0FE
        case 0xA7C8C4: return 0xD26970; // ni_ovl_A7C8C4
        case 0xA7D81A: return 0xD281A0; // ni_ovl_A7D81A
        case 0xA7DBB0: return 0xD28600; // ni_ovl_A7DBB0
        case 0xA7E2DA: return 0xD28FE0; // ni_ovl_A7E2DA
        case 0xA7E37A: return 0xD292A0; // ni_ovl_A7E37A
        case 0xA86CA6: return 0xD35E60; // ni_ovl_A86CA6
        case 0xA92912: return 0xD46850; // ni_ovl_A92912
        case 0xA9B874: return 0xD53570; // ni_ovl_A9B874
        case 0xAA39BA: return 0xD5DC90; // ni_ovl_AA39BA
        case 0xAA8B64: return 0xD65290; // ni_ovl_AA8B64
        case 0xAB029E: return 0xD6F6D0; // ni_ovl_AB029E
        case 0xAB10D4: return 0xD70CA0; // ni_ovl_AB10D4
        case 0xAB6C90: return 0xD780E0; // ni_ovl_AB6C90
        case 0xABAB3E: return 0xD7E960; // ni_ovl_ABAB3E
        case 0xABD154: return 0xD81D90; // ni_ovl_ABD154
        case 0xAD7B1C: return 0xDA6B60; // ni_ovl_AD7B1C
        case 0xAD7B28: return 0xDA6B70; // ni_ovl_AD7B28
        case 0xAE04FC: return 0xDB1C90; // ni_ovl_AE04FC
        case 0xAE615A: return 0xDB91F0; // ni_ovl_AE615A
        case 0xAF1332: return 0xDCA500; // ni_ovl_AF1332
        case 0xAF3D12: return 0xDCE070; // ni_ovl_AF3D12
        case 0xB04E80: return 0xDE5760; // ni_ovl_B04E80
        case 0xB175D4: return 0xE00200; // ni_ovl_B175D4
        case 0xB1B050: return 0xE05600; // ni_ovl_B1B050
        case 0xB27886: return 0xE166E0; // ni_ovl_B27886
        case 0xB31224: return 0xE22AA0; // ni_ovl_B31224
        case 0xB3A734: return 0xE2EB00; // ni_ovl_B3A734
        case 0xB42D50: return 0xE395E0; // ni_ovl_B42D50
        case 0xB44694: return 0xE3B5B0; // ni_ovl_B44694
        case 0xB449CE: return 0xE3BA60; // ni_ovl_B449CE
        case 0xB47542: return 0xE3ED50; // ni_ovl_B47542
        case 0xB484B2: return 0xE40080; // ni_ovl_B484B2
        case 0xB48BE0: return 0xE40A30; // ni_ovl_B48BE0
        case 0xB4F7DC: return 0xE4DD00; // ni_ovl_B4F7DC
        case 0xB50812: return 0xE4F320; // ni_ovl_B50812
        case 0xB5140E: return 0xE509C0; // ni_ovl_B5140E
        case 0xB5166E: return 0xE50D40; // ni_ovl_B5166E
        case 0xB528BA: return 0xE52700; // ni_ovl_B528BA
        case 0xB52BE4: return 0xE52C10; // ni_ovl_B52BE4
        case 0xB52F02: return 0xE53110; // ni_ovl_B52F02
        case 0xB541E0: return 0xE55570; // ni_ovl_B541E0
        case 0xB543C0: return 0xE557B0; // ni_ovl_B543C0
        case 0xB56516: return 0xE58990; // ni_ovl_B56516
        case 0xB57480: return 0xE59CF0; // ni_ovl_B57480
        case 0xB5892A: return 0xE5C1D0; // ni_ovl_B5892A
        case 0xB59058: return 0xE5CF30; // ni_ovl_B59058
        case 0xB594F2: return 0xE5D4D0; // ni_ovl_B594F2
        case 0xB5A130: return 0xE5F2A0; // ni_ovl_B5A130
        case 0xB5A448: return 0xE5F660; // ni_ovl_B5A448
        case 0xB5ADC8: return 0xE60300; // ni_ovl_B5ADC8
        case 0xB5B5D0: return 0xE60DB0; // ni_ovl_B5B5D0
        case 0xB5BA28: return 0xE612A0; // ni_ovl_B5BA28
        case 0xB5BF68: return 0xE61930; // ni_ovl_B5BF68
        case 0xB5C092: return 0xE61AE0; // ni_ovl_B5C092
        case 0xB5C426: return 0xE61F30; // ni_ovl_B5C426
        case 0xB5CDDA: return 0xE62BB0; // ni_ovl_B5CDDA
        case 0xB5D180: return 0xE63080; // ni_ovl_B5D180
        case 0xB5D504: return 0xE63520; // ni_ovl_B5D504
        case 0xB5D8B2: return 0xE639F0; // ni_ovl_B5D8B2
        case 0xB5DBAE: return 0xE63E10; // ni_ovl_B5DBAE
        case 0xB5E214: return 0xE649E0; // ni_ovl_B5E214
        case 0xB5E4DC: return 0xE64E00; // ni_ovl_B5E4DC
        case 0xB5EA7E: return 0xE65460; // ni_ovl_B5EA7E
        case 0xB625C0: return 0xE6B870; // ni_ovl_B625C0
        case 0xB63994: return 0xE6D130; // ni_ovl_B63994
        case 0xB64CCA: return 0xE6E790; // ni_ovl_B64CCA
        case 0xB65D52: return 0xE6FDA0; // ni_ovl_B65D52
        case 0xB66CFC: return 0xE71150; // ni_ovl_B66CFC
        case 0xB67660: return 0xE71F00; // ni_ovl_B67660
        case 0xB67EF2: return 0xE72A20; // ni_ovl_B67EF2
        case 0xB68BF8: return 0xE739E0; // ni_ovl_B68BF8
        case 0xB6A4E0: return 0xE759D0; // ni_ovl_B6A4E0
        case 0xB6B610: return 0xE77290; // ni_ovl_B6B610
        case 0xB6D082: return 0xE79940; // ni_ovl_B6D082
        case 0xB6FF42: return 0xE7D4D0; // ni_ovl_B6FF42
        case 0xB71334: return 0xE7F650; // ni_ovl_B71334
        case 0xB723FC: return 0xE80C90; // ni_ovl_B723FC
        case 0xB73F66: return 0xE830F0; // ni_ovl_B73F66
        case 0xB7534C: return 0xE84AE0; // ni_ovl_B7534C
        case 0xB75C54: return 0xE85760; // ni_ovl_B75C54
        case 0xB776DC: return 0xE87900; // ni_ovl_B776DC
        case 0xB79BFA: return 0xE8AB10; // ni_ovl_B79BFA
        case 0xB7B1FE: return 0xE8C9C0; // ni_ovl_B7B1FE
        case 0xB7C486: return 0xE8E980; // ni_ovl_B7C486
        case 0xB7DC7E: return 0xE90DA0; // ni_ovl_B7DC7E
        case 0xB7E698: return 0xE91A70; // ni_ovl_B7E698
        case 0xB7FBE8: return 0xE93800; // ni_ovl_B7FBE8
        case 0xB8031A: return 0xE94170; // ni_ovl_B8031A
        case 0xB82134: return 0xE96D50; // ni_ovl_B82134
        case 0xB8324E: return 0xE98490; // ni_ovl_B8324E
        case 0xB83E5C: return 0xE995F0; // ni_ovl_B83E5C
        case 0xB84D3E: return 0xE9A950; // ni_ovl_B84D3E
        case 0xB86EBC: return 0xE9DB10; // ni_ovl_B86EBC
        case 0xB8A5F0: return 0xEA2660; // ni_ovl_B8A5F0
        case 0xB8BD92: return 0xEA4460; // ni_ovl_B8BD92
        case 0xB8CC7A: return 0xEA5830; // ni_ovl_B8CC7A
        case 0xB8E04C: return 0xEA76E0; // ni_ovl_B8E04C
        case 0xB91A06: return 0xEAC470; // ni_ovl_B91A06
        case 0xB94830: return 0xEAFBF0; // ni_ovl_B94830
        case 0xB94FB0: return 0xEB04C0; // ni_ovl_B94FB0
        case 0xB954FA: return 0xEB0B10; // ni_ovl_B954FA
        case 0xB963CA: return 0xEB2010; // ni_ovl_B963CA
        case 0xB97AE4: return 0xEB3DB0; // ni_ovl_B97AE4
        case 0xB9A01A: return 0xEB6D10; // ni_ovl_B9A01A
        case 0xB9C88A: return 0xEBA860; // ni_ovl_B9C88A
        case 0xB9FDF4: return 0xEBFB40; // ni_ovl_B9FDF4
        case 0xBA044A: return 0xEC0340; // ni_ovl_BA044A
        case 0xBA1698: return 0xEC1B50; // ni_ovl_BA1698
        case 0xBA2848: return 0xEC31E0; // ni_ovl_BA2848
        case 0xBA2E70: return 0xEC3950; // ni_ovl_BA2E70
        case 0xBA3DF4: return 0xEC4C30; // ni_ovl_BA3DF4
        case 0xBA5B38: return 0xEC74B0; // ni_ovl_BA5B38
        case 0xBA6A4A: return 0xEC8940; // ni_ovl_BA6A4A
        case 0xBA83A2: return 0xECACC0; // ni_ovl_BA83A2
        case 0xBAB4F4: return 0xECFA00; // ni_ovl_BAB4F4
        case 0xBAC4A2: return 0xED0E40; // ni_ovl_BAC4A2
        case 0xBAD290: return 0xED2280; // ni_ovl_BAD290
        case 0xBAE680: return 0xED3FD0; // ni_ovl_BAE680
        case 0xBAF718: return 0xED54C0; // ni_ovl_BAF718
        case 0xBB0712: return 0xED6BA0; // ni_ovl_BB0712
        case 0xBB1352: return 0xED7B80; // ni_ovl_BB1352
        case 0xBB2D88: return 0xEDA5B0; // ni_ovl_BB2D88
        default:
            recomp_printf("[get_decompressed_rom] failed to lookup decompressed ROM offset. Returning input.\n");
            return rom;
    }
}

RECOMP_PATCH void mapOverlay(ObjectHeader* self) {
    s32 temp_t2;
    s32 vaddr;
    u32 size;
    s32 addr;
    u32 i;
    s32 tlb_id;

    recomp_printf("[mapOverlay] called with overlay ID %d 0x%08X and flags 0x%08X\n", self->ID, self->ID, self->flags);

    if (self == 0)
    {
        vaddr = 1;
        addr = 2;
    }

    if (mapped_files_array_index) {
        
    }

    temp_t2 = (s32)(*(u32*)D_8009342C_9402C[(self->ID) & 0x7FF]);
    temp_t2 &= 0x0FFFFFFF;
    addr = D_80387DB8[temp_t2];
    addr &= 0x7FFFFFFF;
    currently_mapped_overlay_evenpaddr[mapped_files_array_index] = addr;
    size = (D_80094838_95438[((temp_t2 - 1))].end) - (D_80094838_95438[((temp_t2 - 1))].start & 0x7FFFFFFF);
    currently_mapped_overlay_size[mapped_files_array_index] = size;
    vaddr = D_80092270_92E70[(self->ID) & 0x7FF] & 0x0F000000;
    currently_mapped_overlay_vaddr[mapped_files_array_index] = vaddr;

    recomp_printf("[mapOverlay] called with addr 0x%08X\n", addr);
    recomp_printf("[mapOverlay] called with size 0x%08X\n", size);
    recomp_printf("[mapOverlay] called with vaddr 0x%08X\n", vaddr);

    switch (((u32) vaddr >> 0x18)) {
    case 0xF: // 0x0F000000
        tlb_id = 0;
        if (size >= 0x36000U) {
            
        }
        break;
    case 0xE: // 0x0E000000
        tlb_id = 0x1B;
        if (size >= 0x8000U) {

        }
        break;
    case 0xC: // 0x0C000000
        tlb_id = 0x10;
        if (size >= 0x16000U) {

        }
        break;
    case 0xD: // 0x0B000000
        tlb_id = 8;
        break;
    }

    for(i = 0; i < size; i += 0x2000, addr += 0x2000) {
        osMapTLB(tlb_id++, 0, (void*)(vaddr + i), addr, addr + 0x1000, -1); // map for both pages for a total of 8KiB
    }

    

    if (self->ID & OBJ_FLAG_MAP_OVERLAY) {
        u32 newID = self->ID;

        // unmask the object bits.
        newID &= ~(OBJ_FLAG_ENABLE_COLLISION);
        newID &= ~(OBJ_FLAG_MAP_OVERLAY);
        newID &= ~(OBJ_FLAG_DESTROY);
        newID &= ~(OBJ_FLAG_MOVE_ALONGSIDE_COLLISION);

        u32 tableID = (s32)(*(u32*)D_8009342C_9402C[(self->ID) & 0x7FF]);
        s32 *fileArr = &D_8009502C_95C2C[tableID << 1];

        u32 rom = fileArr[2] & 0x0FFFFFFF;

        rom = get_decompressed_rom(rom);

        recomp_printf("[mapOverlay] recomp_load_overlays call using args 0x%08X 0x%08X 0x%08X\n", rom, vaddr, size);

        recomp_load_overlays(rom, (void*)vaddr, size);

        /*
        switch (newID) {
            default: // unsupported overlay
                recomp_printf("[mapOverlay] ID not found in lookup. Overlap remapping failed with %d 0x%08X\n", newID, newID);
                break;
        }
        */
    }
    
    mapped_files_array_index++;
}
