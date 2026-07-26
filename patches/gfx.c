#include "patches.h"
#include "misc_funcs.h"
#include "graphics.h"

// game headers

#include "cv64.h"
#include "game/system_work.h"
#include "game/gamestate.h"
#include "gfx/camera.h"
#include "gfx/figure.h"

#include "figure_funcs.h"

// recomp

#include "mtx.h"

// gfx.c

#define gEXMatrixGroupDecomposedNormal(cmd, id, push, proj, edit) \
    gEXMatrixGroupDecomposed(cmd, id, push, proj, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_INTERPOLATE, G_EX_ORDER_LINEAR, edit, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO)

#define gEXMatrixGroupDecomposedNormalVert(cmd, id, push, proj, vert, edit) \
    gEXMatrixGroupDecomposed(cmd, id, push, proj, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, vert, G_EX_COMPONENT_INTERPOLATE, G_EX_ORDER_LINEAR, edit, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO)

RECOMP_PATCH void setup_frame() {
    gDisplayListHead = &sys.graphic_buffers[sys.current_graphic_buffer].dlists;

    gEXEnable(gDisplayListHead++);
    gEXSetRefreshRate(gDisplayListHead++, 30);

    gSPSegment(gDisplayListHead++, 0x00, 0x00000000);
    setup_rsp(&gDisplayListHead);
    if (sys.should_setup_Z_buffer) {
        setup_z_buffer();
    }
    setup_framebuffer();
    if (sys.should_setup_background_color) {
        setup_background_color();
    }
}

extern Mtx* D_80387AE8;

#define FIGURE_ID_MASK 0x8000

struct UnkStruct8000C800_Input {
    char filler0[0x10];
    void *unk10;
}; // unknown size

extern Vp D_80092F58_93B58;

RECOMP_EXPORT int get_vtx_setting_from_id(s16 id) {
    switch(id) {
        // these should be skipped.
        case 0:
        case -1:
            return G_EX_COMPONENT_SKIP;
    }
    return G_EX_COMPONENT_INTERPOLATE;   
}

typedef float Matrix[4][4];

// camera?
RECOMP_PATCH void func_80005684_6284(NewFigure* arg0) {
    int i;

    int figure_idx = (NewFigure*)arg0 - (NewFigure*)figures_array;

    u32 tag = get_tag_from_figure(arg0, figure_idx);

    gSPViewport(gDisplayListHead++, arg0->unk30);

    if (!(arg0->header.type & 0x0100)) {
        gSPPerspNormalize(gDisplayListHead++, arg0->unk24);
    }

    if (gFigureArrMtx[figure_idx].proj != NULL && gFigureArrMtx[figure_idx].view != NULL) {
        // we have a tagged figure which has separated matrices. Use this instead.
        gSPMatrix(gDisplayListHead++, gFigureArrMtx[figure_idx].proj, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
        gSPMatrix(gDisplayListHead++, gFigureArrMtx[figure_idx].view, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION);
    } else {
        // fall back to original logic.
        gSPMatrix(gDisplayListHead++, &D_80387AE8[figure_idx], G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
    }

    for (i = 0; i < 5; i++) {
        //recomp_printf("[func_80005684_6284] gfx buf id %d 0x%08X\n", i, arg0->u.unk3C_gfx[i]);
        *(gDisplayListHead++) = arg0->u.unk3C_gfx[i];
    }
}

extern Mtx* D_80387AE8;

extern void func_80005AD8_66D8(NewFigure *, u32);

static int timer = 0;

RECOMP_PATCH void func_80006194_6D94(NewFigure * arg0) {
    int figure_idx;
    if (arg0->unk38 != NULL) {
        figure_idx = (NewFigure*)arg0->header.prev - (NewFigure*)figures_array;

        gSPMatrix(gDisplayListHead++, &D_80387AE8[figure_idx], G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

        u32 tag = get_tag_from_figure(arg0, figure_idx);

        gEXMatrixGroupDecomposedNormal(gDisplayListHead++, tag, G_MTX_PUSH, G_MTX_MODELVIEW, G_EX_EDIT_NONE);
        func_80005AD8_66D8(arg0, (u32)arg0->unk38);
        gEXPopMatrixGroup(gDisplayListHead++, G_MTX_MODELVIEW);
    }

    figure_idx = (NewFigure*)arg0 - (NewFigure*)figures_array;
    u32 tag = get_tag_from_figure(arg0, figure_idx);

    gSPMatrix(gDisplayListHead++, &D_80387AE8[figure_idx], G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

    if (arg0->unk30 != NULL) {
        u32 tag = get_tag_from_figure(arg0, figure_idx);

        gEXMatrixGroupDecomposedNormal(gDisplayListHead++, tag, G_MTX_PUSH, G_MTX_MODELVIEW, G_EX_EDIT_NONE);
        func_80005AD8_66D8(arg0, (u32)arg0->unk30);
        gEXPopMatrixGroup(gDisplayListHead++, G_MTX_MODELVIEW);
    }

    tag = get_tag_from_figure(arg0, figure_idx);

    Object *hud = get_hud_object();
    enum HudObjectType hud_object_type = check_figure_for_hud(arg0, hud);

    static int pop_viewport = 0;

    // only align HUD if we are not in pause menu.
    if ((is_pause_menu_spawned() == 0) && (hud_object_type == HUD_OBJECT_CAMERA_MODE 
            || hud_object_type == HUD_OBJECT_BAR 
            || hud_object_type == HUD_OBJECT_SUBWEAPON)) {
#define ALIGN_UI_LEFT  0
#define ALIGN_UI_RIGHT 1
        int alignType;

        switch(hud_object_type) {
            case HUD_OBJECT_BAR:
            case HUD_OBJECT_CAMERA_MODE:
                alignType = ALIGN_UI_LEFT;
                break;
            case HUD_OBJECT_SUBWEAPON:
                alignType = ALIGN_UI_RIGHT;
                break;
        }

        gEXPushViewport(gDisplayListHead++);
        gEXPushScissor(gDisplayListHead++);

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

        // inverted this check so it will use the right align on the bar
        if (alignType == ALIGN_UI_LEFT) {
            gEXSetScissor(gDisplayListHead++, G_SC_NON_INTERLACE, G_EX_ORIGIN_LEFT, G_EX_ORIGIN_LEFT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            gEXSetViewportAlign(gDisplayListHead++,G_EX_ORIGIN_LEFT, 0, 0); 
            gSPViewport(gDisplayListHead++, (void*)&D_80092F58_93B58);
        } else {
            gEXSetScissor(gDisplayListHead++, G_SC_NON_INTERLACE, G_EX_ORIGIN_RIGHT, G_EX_ORIGIN_RIGHT, -SCREEN_WIDTH, 0, 0, SCREEN_HEIGHT);
            gEXSetViewportAlign(gDisplayListHead++, G_EX_ORIGIN_RIGHT, -320 * 4, 0); 
            gSPViewport(gDisplayListHead++, (void*)&D_80092F58_93B58);
        }
        pop_viewport = 1;
    } else {
        pop_viewport = 0;
    }

    if (tag && arg0->header.type & FIG_TYPE_ALLOW_TRANSPARENCY_CHANGE) {
        recomp_printf("[func_80006194_6D94] (%d) Type flags for figure 0x%08X allowed transparent change! tag 0x%08X\n", timer++, arg0->header.type, tag);
    }

    int vert = get_vtx_setting_from_id(get_id_from_figure(arg0, figure_idx));
    gEXMatrixGroupDecomposedNormalVert(gDisplayListHead++, tag, G_MTX_PUSH, G_MTX_MODELVIEW, vert, G_EX_EDIT_NONE);
    func_80005AD8_66D8(arg0, (u32)arg0->unk34);
    gEXPopMatrixGroup(gDisplayListHead++, G_MTX_MODELVIEW);

    if (pop_viewport) {
        gEXPopScissor(gDisplayListHead++);
        gEXPopViewport(gDisplayListHead++);
    }
}

// HUD buffer

#define	ABS(d)		(((d) >= 0) ? (d) : -(d))

extern s16 D_80387AC0;

struct UnkStruct_func_8012E8A0_B1A90_Arg4 {
    s32 unk0;
    char pad4[0x28];
    u16 unk2C; // tile
    u8 pad2E[0x2];
    u8 unk30;
    u8 pad31[0xB];
    s8 unk3C[10][2];
};

struct UnkStruct_func_8012E8A0_B1A90_Arg5 {
    char pad0[0x12];
    u16 unk12;
    u16 unk14;
};

struct UnkStruct_func_8012E8A0_B1A90_Arg1 {
    u32 unk0;
    u32 unk4;
    u32 unk8;
};

// @recomp The things we do for recomp.
//
// The function below uses a fixed-gfx buffer which the game assumes certain data is located at certain offsets
// into the print/gfx buffer. As such, if we were to try to append the EX commands directly via a patch, the
// offsets used later on by the game no longer match and the game will crash.
// 
// To resolve this, we move a total of 70 bytes (to save 64) to an external branch and branch to this to save memory, which
// is enough for the EX push/pop and rect align. No-ops are added otherwise to preserve the size.
Gfx func_8012E8A0_Gfx_patch[] = {
    gsDPPipeSync(), // 8 bytes (1 cmd)
    gsDPSetCycleType(G_CYC_2CYCLE), // 8 bytes (1 cmd)
    gsDPSetRenderMode(G_RM_PASS, G_RM_XLU_SURF2), // 8 bytes (1 cmd)
    gsDPSetAlphaCompare(G_AC_THRESHOLD), // 8 bytes (1 cmd)
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01), // 8 bytes (1 cmd)
    gsSPClearGeometryMode(G_CULL_BOTH | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR), // 8 bytes (1 cmd)
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON), // 8 bytes (1 cmd)
    gsDPSetTextureLOD(G_TL_TILE), // 8 bytes (1 cmd)
    gsDPSetTextureDetail(G_TD_CLAMP), // 8 bytes (1 cmd)
    gsSPEndDisplayList(),
};

// print text to x/y, wrap lines
RECOMP_PATCH void func_8012E8A0_B1A90(Gfx** arg0, struct UnkStruct_func_8012E8A0_B1A90_Arg1 arg1, struct UnkStruct_func_8012E8A0_B1A90_Arg4* arg4, struct UnkStruct_func_8012E8A0_B1A90_Arg5* arg5) {
    Gfx *gfx;
    Gfx *temp_s3 = arg0[D_80387AC0 + 1];
    s8 var_s4;
    s16 var_t4 = arg5->unk14;
    u8* temp_v0;
    s8 temp_t2;
    s8 temp_t0;
    s32 var_a1;
    s16 var_t3;
    s16 var_a2;
    s16 var_t5;
    int should_right_align = 0;
    int i = 0;

    // consolidate 70-bytes worth of GFX commands into an 8 byte display list CMD, saving 64 bytes.
    // see @recomp note above.
    gSPDisplayList(&temp_s3[i++], func_8012E8A0_Gfx_patch);

    //gDPPipeSync(&temp_s3[0]); // 8 bytes (1 cmd)
    //gDPSetCycleType(&temp_s3[1], G_CYC_2CYCLE); // 8 bytes (1 cmd)
    //gDPSetRenderMode(&temp_s3[2], G_RM_PASS, G_RM_XLU_SURF2); // 8 bytes (1 cmd)
    //gDPSetAlphaCompare(&temp_s3[3], G_AC_THRESHOLD); // 8 bytes (1 cmd)
    //gDPSetBlendColor(&temp_s3[4], 0x00, 0x00, 0x00, 0x01); // 8 bytes (1 cmd)
    //gSPClearGeometryMode(&temp_s3[5], G_CULL_BOTH | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR); // 8 bytes (1 cmd)
    //gSPSetGeometryMode(&temp_s3[6], G_CULL_BACK | G_FOG); // 8 bytes (1 cmd)
    //gSPTexture(&temp_s3[i++], 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON); // 8 bytes (1 cmd)
    //gDPSetTextureLOD(&temp_s3[i++], G_TL_TILE); // 8 bytes (1 cmd)
    //gDPSetTextureDetail(&temp_s3[i++], G_TD_CLAMP); // 8 bytes (1 cmd)

    gDPSetTexturePersp(&temp_s3[i++], G_TP_NONE);
    gDPSetAlphaDither(&temp_s3[i++], G_AD_DISABLE);
    gDPSetCombineLERP(&temp_s3[i++], 0, 0, 0, TEXEL0, ENVIRONMENT, 0, TEXEL1, TEXEL0, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED);
    gDPSetEnvColor(&temp_s3[i++], 0x00, 0x00, 0x00, 0x02);
    gDPSetTextureLUT(&temp_s3[i++], G_TT_RGBA16);
    gDPSetTextureFilter(&temp_s3[i++], G_TF_POINT);
    gDPSetTile(&temp_s3[i++], G_IM_FMT_CI, G_IM_SIZ_4b, arg4->unk2C >> 4, 0, 1, 1, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOLOD);
    gDPSetTileSize(&temp_s3[i++], 1, 0, 0, arg4->unk2C << 2, 0x0040);
    temp_v0 = (u8*)temp_s3 + arg1.unk4;
    gDPSetTextureImage(&temp_s3[i++], G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, temp_v0);\
    gDPTileSync(&temp_s3[i++]);\
    gDPSetTile(&temp_s3[i++], G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 0x0100, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);\
    gDPLoadSync(&temp_s3[i++]);\
    gDPLoadTLUTCmd(&temp_s3[i++], G_TX_LOADTILE, 15);\
    gDPPipeSync(&temp_s3[i++]);
    gDPSetTextureImage(&temp_s3[i++], G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, temp_v0 + 0x20);\
    gDPTileSync(&temp_s3[i++]);\
    gDPSetTile(&temp_s3[i++], G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 0x0110, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);\
    gDPLoadSync(&temp_s3[i++]);\
    gDPLoadTLUTCmd(&temp_s3[i++], G_TX_LOADTILE, 15);\
    gDPPipeSync(&temp_s3[i++]);

    //recomp_printf("[func_8012E8A0_B1A90] x 0x%08X y 0x%08X\n", arg5->unk12, arg5->unk14);

    if ((is_pause_menu_spawned() == 0) && arg5->unk12 == 0x10E && arg5->unk14 == 0x27) {
        should_right_align = 1;
    } else if ((is_pause_menu_spawned() == 0) && arg5->unk12 == 0xE4 && arg5->unk14 == 0x19) { 
        should_right_align = 1;
    }

    var_s4 = 0;
    gfx = &temp_s3[i++];

    if (should_right_align) {
        /* +x08 */ gEXPushScissor(gfx++);
        /* +x16 */ gEXSetScissor(gfx++, 0, G_EX_ORIGIN_RIGHT, G_EX_ORIGIN_RIGHT, -SCREEN_WIDTH, 0, 0, SCREEN_HEIGHT);
        /* +x16 */ gEXSetRectAlign(gfx++, G_EX_ORIGIN_RIGHT, G_EX_ORIGIN_RIGHT, -SCREEN_WIDTH * 4, 0, -SCREEN_WIDTH * 4, 0);
    } else {
        // should push 40 bytes.
        /* +x08 */ gEXNoOp(gfx++);
        /* +x08 */ gEXNoOp(gfx++);
        /* +x08 */ gEXNoOp(gfx++);
        /* +x08 */ gEXNoOp(gfx++);
        /* +x08 */ gEXNoOp(gfx++);
    }

    while (1) {
        if ((var_s4 == arg4->unk30) || (var_s4 == 0xA)) {
            gDPSetAlphaCompare(gfx++, G_AC_NONE);
            gDPSetTexturePersp(gfx++, G_TP_PERSP);
            if (should_right_align) {
                /* +x16 */ gEXSetRectAlign(gfx++, G_EX_ORIGIN_NONE, G_EX_ORIGIN_NONE, 0, 0, 0, 0);
                /* +x08 */ gEXPopScissor(gfx++);
            } else {
                // should push 24 bytes.
                /* +x08 */ gEXNoOp(gfx++);
                /* +x08 */ gEXNoOp(gfx++);
                /* +x08 */ gEXNoOp(gfx++);
            }
            gSPEndDisplayList(gfx++);
            break;
        }
        
        gDPPipeSync(gfx++);
        gDPLoadTextureBlock_4b(gfx++, ((u8*)temp_s3 + arg1.unk0 + (((arg4->unk2C >> 1) + (arg4->unk2C & 1)) * 0x10 * var_s4)), G_IM_FMT_CI, arg4->unk2C, 16, 0, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, 8, 4, G_TX_NOLOD, G_TX_NOLOD);

        temp_t2 = (arg4->unk3C[var_s4][1] / 2) * 2;
        temp_t0 = (arg4->unk3C[var_s4][0] / 2) * 2;
        if (temp_t2 > 0) {
            var_t5 = arg5->unk12 - ((arg4->unk2C * temp_t2) / 2);
            var_a2 = (var_a1 = arg5->unk12 + arg4->unk2C) + ((arg4->unk2C * temp_t2) / 2);
        } else if (temp_t2 < 0) {
            var_t5 = arg5->unk12 + ((arg4->unk2C - (arg4->unk2C / ABS(temp_t2))) / 2);
            var_a1 = arg5->unk12 + arg4->unk2C;
            var_a2 = var_a1 - ((arg4->unk2C - (arg4->unk2C / ABS(temp_t2))) / 2);
        } else {
            var_a1 = arg5->unk12 + arg4->unk2C;
            var_t5 = arg5->unk12;
            var_a2 = var_a1;
        }

        if (temp_t0 > 0) {
            var_t3 = var_t4 + (temp_t0 * 16);
        } else if (temp_t0 < 0) {
            var_t3 = var_t4 + (16 / ABS(temp_t0));
        } else {
            var_t3 = var_t4 + 16;
        }
        

        if (arg4->unk0 & 8) {
            var_t5 = arg5->unk12;\
            var_a2 = var_a1;
            if (temp_t0 > 0) {
                var_t4 -= ((temp_t0 * 16) - 16) / 2;
                var_t3 -= ((temp_t0 * 16) - 16) / 2;
            } else if (temp_t0 < 0) {
                var_t4 += (16 - (16 / ABS(temp_t0))) / 2;
                var_t3 += (16 - (16 / ABS(temp_t0))) / 2;
            }
        }

        gSPScisTextureRectangle(gfx++, var_t5 << 2, var_t4 << 2, var_a2 << 2, var_t3 << 2, 0, 0, 0, (1 << (10 - (temp_t2 / 2))), (1 << (0xA - (temp_t0 / 2))));
        var_t4 = var_t3;
        var_s4++;
    }
}
