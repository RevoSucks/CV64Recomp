#include "patches.h"
#include "misc_funcs.h"
#include "graphics.h"

#include "cv64.h"

extern Gfx Another_Display_List[];
extern u8 DAT_800a78f0[];
extern u8 D_800A78E0_A8460[]; // unk

extern Gfx* gDisplayListHead;

struct UnkStruct80383AB8 {
    char filler0[0x4028];
    u16 unk4028;
    u16 unk402A;
};

extern struct UnkStruct80383AB8 D_80383AB8;

RECOMP_PATCH void func_80153908_D6AF8(Gfx** gfxP) {
    gDPPipeSync((*gfxP)++);
    gDPSetTexturePersp((*gfxP)++, G_TP_PERSP);
    gDPSetTextureFilter((*gfxP)++, G_TF_BILERP);
    gDPSetAlphaCompare((*gfxP)++, G_AC_NONE);

    recomp_printf("[func_80153908_D6AF8] fog position 0x%08X 0x%08X\n", D_80383AB8.unk4028, D_80383AB8.unk402A);
    gSPFogPosition((*gfxP)++, D_80383AB8.unk4028, D_80383AB8.unk402A);
}

RECOMP_PATCH void drawFog(void) {
    gSPLookAtX(gDisplayListHead++, D_800A78E0_A8460);
    gSPLookAtY(gDisplayListHead++, DAT_800a78f0);
    gSPDisplayList(gDisplayListHead++, Another_Display_List);
    gSPFogPosition(gDisplayListHead++, D_80383AB8.unk4028, D_80383AB8.unk402A);
    gDPSetFogColor(gDisplayListHead++, 0x00, 0x00, 0x00, 0xFF);
    func_80153908_D6AF8(&gDisplayListHead);
}
