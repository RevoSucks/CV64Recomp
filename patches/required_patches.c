#include "patches.h"
#include "misc_funcs.h"
#include "PR/os_pi.h"

unsigned long long dummy = 0x0123456789ABCDEFULL;

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

// Really shitty workaround.
extern OSPiHandle* gCartHandle;
extern void DMA_readWrite(OSPiHandle* piHandle, s32 direction, void* dest, void* src, s32 len);
extern void overlay_apply_relocations(u32 file_id, u8 *load_addr);

RECOMP_PATCH void DMA_ROMCopy(void* src, void* dest, s32 len) {
    u32 rom_addr = (u32)src;

    recomp_printf("[DMA_ROMCopy] load src 0x%08X dest 0x%08X len 0x%08X\n", rom_addr, (u32)dest, len);

    // hardcoded size for test. TODO: fix
    switch(rom_addr) {
        case 0xB5140E: 
            recomp_load_overlays(0xE509C0, (void*)0x0F000000, 0x380); 
            overlay_apply_relocations(389, (void*)0x0F000000);
            break;
    }

    DMA_readWrite(gCartHandle, OS_READ, dest, src, len);
}

extern void romCopyAndDecompress(u32, void *, u32);

// load_compressed_file
RECOMP_PATCH void *loadCompressedFile(u32 file_id, u8* buf_start) {
    s32 cur_file_id;
    s32 next_file_id;
    u32 file_rom_addr;
    u32 file_size;
    u32 file_size_aligned;
    u32 vram_start;
    u32 vram_end;
    u8 *buf_cur;
    u8 *buf_end;
    s32 *fileArr = &D_8009502C_95C2C[file_id << 1];

    recomp_printf("[loadCompressedFile] file_id 0x%08X buf_start 0x%08X\n", file_id, buf_start);

    //was_loading_file = D_8015C5D4_15D1D4;

    if (file_id < FILE_MAX && file_id != FILE_NONE) {
        cur_file_id = file_id - 1;
        next_file_id = file_id;
        file_rom_addr = fileArr[2] & FILE_ADDR_MASK;
        file_size = fileArr[-2 + 5] - file_rom_addr;
        vram_start = D_80094838_95438[cur_file_id].start;
        vram_end = D_80094838_95438[cur_file_id].end;

        buf_end = buf_start + ((s32)vram_end - (s32)vram_start);
        recomp_printf("[loadCompressedFile] vram_start 0x%08X vram_end 0x%08X\n", vram_start, vram_end);

        //D_8015C5D4_15D1D4 = TRUE;
        
        buf_cur = buf_start;

        if (file_size != 0) {
            // @recomp Load the overlay in the recomp runtime.
            recomp_printf("[loadCompressedFile] Calling recomp_load_overlays with arguments 0x%08X 0x%08X 0x%08X\n", file_rom_addr, buf_start, file_size);
            recomp_load_overlays(file_rom_addr, (void *)buf_start, file_size);
            
            if (D_8009502C_95C2C[cur_file_id] & FILE_COMP_MASK) {
                DMA_ROMCopy((void*)file_rom_addr, buf_start, file_size);
            } else {
                file_size_aligned = (file_size + 1) & ~1;
                romCopyAndDecompress(file_rom_addr, (void *)buf_start, file_size_aligned);
                buf_cur = buf_start + file_size_aligned;
            }

            recomp_printf("[loadCompressedFile] buf_cur 0x%08X buf_start 0x%08X buf_end 0x%08X\n", buf_cur, buf_start, buf_end);

            // @recomp Apply relocations after loading the overlay.
            // TODO
        }

        //D_8015C5D4_15D1D4 = was_loading_file;

        /*
        while (buf_cur < buf_end) {
            *buf_cur = 0;
            buf_cur++;
        }
        */

        // @recomp Not needed in the static recompilation.
        // osWritebackDCache_recomp(buf_start, buf_end - buf_start);
    } else {
        buf_end = NULL;
    }

    // Sometimes used to determine where to next load a file.
    return buf_end;
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

extern u32 *D_8009342C_9402C[];

struct UnkStruct80094838 {
    u32 unk0[1];
};

extern u32 D_80092270_92E70[];

RECOMP_PATCH void mapOverlay(s16* arg0) {
    s32 temp_t2;
    s32 addr;
    s32 tlb_id;
    u32 size;
    u32 vaddr;
    u32 i;

    temp_t2 = (*D_8009342C_9402C[*arg0 & 0x7FF] & 0x0FFFFFFF);
    addr = D_80387DB8[(temp_t2)];
    currently_mapped_overlay_evenpaddr[mapped_files_array_index] = addr & 0x7FFFFFFF;
    size = ((u32*)D_80094838_95438)[(temp_t2 << 1) - 1] - (((u32*)D_80094838_95438)[(temp_t2 << 1) - 2] & 0x7FFFFFFF);
    currently_mapped_overlay_size[mapped_files_array_index] = size;
    vaddr = D_80092270_92E70[*arg0 & 0x7FF] & 0x0F000000;
    currently_mapped_overlay_vaddr[mapped_files_array_index] = vaddr;

    recomp_printf("[mapOverlay] addr 0x%08X size 0x%08X vaddr 0x%08X\n", addr, size, vaddr);

    recomp_printf("[mapOverlay] id should be %d 0x%02X\n", *arg0 & 0x7FF, *arg0 & 0x7FF);
    recomp_printf("[mapOverlay] vaddr should be 0x%08X\n", D_80092270_92E70[*arg0 & 0x7FF]);
    
    switch ((vaddr >> 0x18)) {
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

    for(i = 0; i < size; i += 0x2000, vaddr += 0x2000, addr += 0x2000) {
        osMapTLB(tlb_id++, 0, (void*)vaddr, addr, addr + 0x1000, -1); // map for both pages for a total of 8KiB
    }
    
    mapped_files_array_index++;
}

