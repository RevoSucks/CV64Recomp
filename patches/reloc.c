#include "patches.h"
#include "relocs.h"

void overlay_apply_relocations(u32 file_id, u8 *load_addr) {
    recomp_printf("[overlay_apply_relocations] file_id 0x%08X load_addr 0x%08X\n", file_id, load_addr);

    if (file_id >= RELOC_TABLE_SIZE) {
        return;
    }

    const RelocInfo *info = &g_relocs[file_id];
    if (info->offsets == NULL || info->count == 0) {
        recomp_printf("[overlay_apply_relocations] WARNING: relocs not found for file ID %d 0x%08X\n", file_id, file_id);
        return;
    }

    const s32 delta = (s32)((u32)load_addr - info->original_vaddr);
    if (delta == 0) {
        return;
    }

    for (int i = 0; i < info->count; i++) {
        u32 offset = info->offsets[i];
        u32 *value_addr = (u32 *)(load_addr + offset);
        u32 value = *value_addr;

        *value_addr = value + (u32)delta;

        recomp_printf("[overlay_apply_relocations] 0x%08X -> 0x%08X\n", value, *value_addr);
    }
}