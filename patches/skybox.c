#include "patches.h"
#include "misc_funcs.h"
#include "graphics.h"

#include "game/object.h"

extern s32 func_80157BD4_DADC4(void*);                     /* extern */

struct Figure_Skybox_Inner_unk34 {
    char filler0[0x18];
    f32 unk18[2];
    f32 unk20[2];
    u8 unk28;
    u8 unk29;
    u8 unk2A;
    s16 unk2C;
};

struct Figure_Skybox_Inner_unk34_Addr {
    // offsetted at +0x34 of skybox figure
    struct Figure_Skybox_Inner_unk34 *unk34;
    u8 filler38[0xC];
    f32 unk10[2];
    f32 unk18[2];
    u32 unk54;
    u8 filler58[0x4];
    s16 unk5C;
};

struct Figure_Skybox_Inner_sp24 {
    char filler0[0x30];
    u32 unk30;
};

// Generic figure struct
typedef struct Figure_Skybox {
    FigureHeader header;
    u8 filler14[0x10];
    void *unk24;
    u8 filler28[0xC];
    struct Figure_Skybox_Inner_unk34_Addr unk34;
} Figure_Skybox; // Size = 0xA8

struct UnkStruct80383AB8 {
    char filler0[0x4028];
    u16 unk4028;
    u16 unk402A;
    char filler402C[0x54];
    f32 unk4080; // changed from s16
    char filler4084[0x40A0 - 0x4084];
    f32 unk40A0;
    char filler40A4[0x4];
    f32 unk40A8[3];
    char filler40B4[0x6414-0x40B4];
    u8 unk6414;
    u8 unk6415;
    u8 unk6416;
};

extern struct UnkStruct80383AB8 D_80383AB8;

// skybox_animate
RECOMP_PATCH void func_8015B178_DE368(Object* arg0) {
    struct Figure_Skybox_Inner_unk34_Addr *temp_a3 = &arg0->alloc_data[0];
    struct Figure_Skybox_Inner_unk34 *temp_s0 = temp_a3->unk34;
    struct Figure_Skybox_Inner_sp24* sp24;
    int i;

    sp24 = arg0->figures[0];

    temp_s0->unk28 = (u8) D_80383AB8.unk6414;
    temp_s0->unk29 = (u8) D_80383AB8.unk6415;
    temp_s0->unk2A = (u8) D_80383AB8.unk6416;

    for (i = 0; i < 2; i++) {
        temp_s0->unk18[i] += temp_a3->unk10[i];
        temp_s0->unk20[i] += temp_a3->unk18[i];

        if (temp_a3->unk10[i] < 0.0) {
            if (temp_s0->unk18[i] < 0.0) {
                temp_s0->unk18[i] += 64.0f;
            }
        } else {
            if (temp_s0->unk18[i] > 64.0f) {
                temp_s0->unk18[i] -= 64.0f;
            }
        }

        if (temp_a3->unk18[i] < 0.0) {
            if (temp_s0->unk20[i] < 0.0) {
                temp_s0->unk20[i] += 64.0f;
            }
        } else {
            if (temp_s0->unk20[i] > 64.0f) {
                temp_s0->unk20[i] -= 64.0f;
            }
        }
    }

    temp_a3->unk54 += 0x10;
    temp_a3->unk54 &= 0xFFFF;
    temp_s0->unk2C = ((sins(temp_a3->unk54) / 512) + 0x80);

    // first, simulate the coords being passed to the command.
    int x1 = (int)(temp_s0->unk18[0] * 4.0f);
    int y1 = (int)(temp_s0->unk20[0] * 4.0f);
    int x2 = (int)(temp_s0->unk18[1] * 4.0f);
    int y2 = (int)(temp_s0->unk20[1] * 4.0f);

    float x1_float = (temp_s0->unk18[0] * 4.0f);
    float y1_float = (temp_s0->unk20[0] * 4.0f);
    float x2_float = (temp_s0->unk18[1] * 4.0f);
    float y2_float = (temp_s0->unk20[1] * 4.0f);

    float x1_frac = x1_float - (float)x1;
    float y1_frac = y1_float - (float)y1;
    float x2_frac = x2_float - (float)x2;
    float y2_frac = y2_float - (float)y2;

    recomp_printf("[func_8015B178_DE368] x1_frac %f y1_frac %f x2_frac %f y2_frac %f\n", x1_frac, y1_frac, x2_frac, y2_frac);

    gEXSetTileScrollFloat(gDisplayListHead++, 0, x1_frac, y1_frac);
    gEXSetTileScrollFloat(gDisplayListHead++, 1, x2_frac, y2_frac);
    sp24->unk30 = func_80157BD4_DADC4(temp_s0);
    gEXSetTileScrollFloat(gDisplayListHead++, 0, 0, 0);
    gEXSetTileScrollFloat(gDisplayListHead++, 1, 0, 0);
}
