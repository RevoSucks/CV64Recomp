#include "patches.h"
#include "misc_funcs.h"

// hud.c

struct UnkStruct_HUD_updateGameplayHUDPos_Input_Unk24 {
    s16 unk0;
    u16 unk2;
    char filler4[0x3C];
    f32 unk40;
    f32 unk44;
};

struct UnkStruct_HUD_updateGameplayHUDPos_Input_Unk70_Unk28 {
    char filler0[0x14];  
    u16 unk14;
    u16 unk16;
};

struct UnkStruct_HUD_updateGameplayHUDPos_Input_Unk70 {
    u8 unk0;
    char filler1[0x23];
    void *unk24;
    struct UnkStruct_HUD_updateGameplayHUDPos_Input_Unk70_Unk28 *unk28;
    struct UnkStruct_HUD_updateGameplayHUDPos_Input_Unk70_Unk28 *unk2C;
};

struct UnkStruct_HUD_updateGameplayHUDPos_Input {
    char filler0[0x24];
    struct UnkStruct_HUD_updateGameplayHUDPos_Input_Unk24 *unk24;
    char filler28[0x4];
    struct UnkStruct_HUD_updateGameplayHUDPos_Input_Unk24 *unk2C;
    struct UnkStruct_HUD_updateGameplayHUDPos_Input_Unk24 *unk30;
    char filler34[0x24];
    struct UnkStruct_HUD_updateGameplayHUDPos_Input_Unk24 *unk58;
    char filler5C[0x10];
    s8 unk6C;
    struct UnkStruct_HUD_updateGameplayHUDPos_Input_Unk70 *unk70;
};

extern f32 D_801804B4_1036A4[][2];

RECOMP_PATCH void HUD_updateGameplayHUDPos(struct UnkStruct_HUD_updateGameplayHUDPos_Input* arg0, s32 arg1) {
    f32* temp_a1;
    u8 temp_a3;
    s32 temp_a2;
    struct UnkStruct_HUD_updateGameplayHUDPos_Input_Unk70* temp_v0;
    struct UnkStruct_HUD_updateGameplayHUDPos_Input_Unk24 *temp_v1;
    struct UnkStruct_HUD_updateGameplayHUDPos_Input_Unk70_Unk28 *temp_v1_2;
    int hudMode = temp_v0->unk0 & 1;

    temp_v0 = arg0->unk70;
    temp_a2 = temp_v0->unk0;
    temp_a3 = ((temp_a2 & 1) * 6) & 0xFF;

    recomp_printf("[HUD_updateGameplayHUDPos] HUD pos mode %d\n", temp_a3);

    if ((((temp_a2 & 2) >> 1) != temp_a3) || (arg1 != 0)) {
        temp_v0->unk0 &= 0xFFFD; \
        temp_v0->unk0 |= (temp_v0->unk0 & 1) << 1;

        temp_a1 = D_801804B4_1036A4[temp_a3];

        temp_v1 = arg0->unk24;
        temp_v1->unk2 &= 0xBFFF;

        recomp_printf("[HUD_updateGameplayHUDPos] HUD pos 1 %.6f (addr 0x%08X)\n", temp_a1[0], (u32)temp_a1);
        recomp_printf("[HUD_updateGameplayHUDPos] HUD pos 2 %.6f (addr 0x%08X)\n", temp_a1[1], (u32)temp_a1);

        temp_v1->unk40 = temp_a1[0];
        temp_v1->unk44 = temp_a1[1];

        temp_v1 = arg0->unk2C;
        temp_v1->unk2 &= 0xBFFF;
        temp_v1->unk40 = temp_a1[2];
        temp_v1->unk44 = temp_a1[3];
        
        temp_v1 = arg0->unk58;
        temp_v1->unk2 &= 0xBFFF;
        temp_v1->unk40 = temp_a1[4];
        temp_v1->unk44 = temp_a1[5];

        temp_v1 = arg0->unk30;
        temp_v1->unk2 &= 0xBFFF;
        temp_v1->unk40 = temp_a1[6];
        temp_v1->unk44 = temp_a1[7];

        temp_v1_2 = temp_v0->unk28;
        temp_v1_2->unk14 = temp_a1[8];
        temp_v1_2->unk16 = temp_a1[9];

        temp_v1_2 = temp_v0->unk2C;
        temp_v1_2->unk14 = temp_a1[10];
        temp_v1_2->unk16 = temp_a1[11];
        arg0->unk6C = 0xA;
        return;
    }
    
    if (arg0->unk24->unk0 >= 0) {
        if (arg0->unk6C > 0) {
            arg0->unk6C--;
            return;
        }
    }

    if (arg0->unk6C == 0) {
        arg0->unk24->unk2 |= 0x4000;
        arg0->unk2C->unk2 |= 0x4000;
        arg0->unk58->unk2 |= 0x4000;
        arg0->unk30->unk2 |= 0x4000;
        arg0->unk6C = -1;
    }
}
