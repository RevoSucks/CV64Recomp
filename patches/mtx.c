#include "patches.h"
#include "misc_funcs.h"
#include "graphics.h"

#include "mtx.h"

void func_80002A98_3698(s32*, f32*, f32*);               /* extern */
void func_80002DA8_39A8(f32*, void*, void*); // changed return type from s32
void func_8000CA9C_D69C(f32*, f32*, f32*);      /* extern */
void func_8000CB50_D750(f32*, void *);                    /* extern */
extern f64 D_800A2790_A3390;
extern f64 D_800A2798_A3398;
extern Mtx* D_80387AE8;
extern f32 D_80387B30[4];
extern f32 D_80387B38;
extern f32 D_80387B58;
extern s32 D_80389E8C;

typedef struct FigureHeader {
    s16 type;
    u16 flags;
    struct FigureHeader* prev;
    struct FigureHeader* sibling;
    struct FigureHeader* next;
    struct FigureHeader* parent;
} FigureHeader; // Size = 0x14

// Generic figure struct
typedef struct NewFigure {
    FigureHeader header;
    u8 field_0x14[0x24 - 0x14];
    /* 0x24 */ u16 unk24;
    /* 0x26 */ u16 unk26;
    /* 0x28 */ u32 *unk28;
    char filler2C[0x4];
    /* 0x30 */ void *unk30;
    f32 *unk34; // changed from void*
    void *unk38;
    union Unk3CUnion {
        Gfx *unk3C_gfx;
        u32 unk3C_u32;
    } u;
    u32 unk40;
    u8 filler44[0x8];
    u32 unk4C;
    u8 filler50[0x2];
    s16 unk52;
    s16 unk54;
    s16 unk56;
    f32 unk58;
    f32 unk5C;
    f32 unk60;
    u8 filler64[0x4];
    u32 unk68;
    u8 filler6C[0x2C];
    f32 unk98;
    f32 unk9C;
    f32 unkA0;
    s32 fillerA4;
} NewFigure; // Size = 0xA8

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
};

extern struct UnkStruct80383AB8 D_80383AB8;

struct UnkStruct80093360 {
    s32 unk0;  
    s32 unk4;
    s32 unk8;
};

extern struct UnkStruct80093360 D_80093360_93F60;
// changed extern s32 atan2f(f32 arg1, f32 arg2);

extern void Figure_UpdateMatrices(void*);
extern void memory_copy(void* src, void* dest, u32 size);
extern void* common_camera_game_view;

extern NewFigure figures_array[];

// the game does not return f32s, it returns signed 32-bits. what? it's literally in your name.

// we cant use the decomp repo's definition of this as it's somehow incorrect.
extern s32 atan2f(f32 arg1, f32 arg2);

// if the ptrs are set, the figure has separate view and projection
// matrices. This will be used instead of the multiplied one on the CPU.

// 0 = view
// 1 = proj

#define FIGURE_MTX_ARRAY_VIEW 0
#define FIGURE_MTX_ARRAY_PROJ 1

Matrix gFigureMatrixArray[FIG_ARRAY_MAX][2]; // this is what is pointed to. Hope this doesnt overflow...

struct FigureArrMtxSep gFigureArrMtx[FIG_ARRAY_MAX] = {0};

extern void guMtxF2L(float mf[4][4], Mtx *m);

RECOMP_EXPORT void set_figure_arr_matrix(int figure_idx, Matrix mtx, int type) {
    guMtxF2L(mtx, &gFigureMatrixArray[figure_idx][type]);
    
    if (type == FIGURE_MTX_ARRAY_VIEW) {
        gFigureArrMtx[figure_idx].view = &gFigureMatrixArray[figure_idx][FIGURE_MTX_ARRAY_VIEW];
    } else {
        gFigureArrMtx[figure_idx].proj = &gFigureMatrixArray[figure_idx][FIGURE_MTX_ARRAY_PROJ];
    }
}

RECOMP_PATCH void func_8000CC50_D850(NewFigure* figure) {
    Matrix sp80;
    f32 sp7C;
    f32 sp78;
    f32 sp74;
    f32 *temp_v0;
    s32 pad;
    NewFigure *temp2;
    NewFigure *temp; // sp64
    struct UnkStruct80093360 sp58;

    if (figure->header.type & 0x100) {
        temp_v0 = figure->unk34;
        guOrthoF(sp80, temp_v0[0], temp_v0[1], temp_v0[2], temp_v0[3], temp_v0[4], temp_v0[5], temp_v0[6]);
    } else {
        temp_v0 = figure->unk34;
        guPerspectiveF(sp80, &figure->unk24, temp_v0[0], temp_v0[1], temp_v0[2], temp_v0[3], temp_v0[4]);
    }

    if (figure->header.flags & 0x60) {
        func_80002DA8_39A8(&figure->unk68, &figure->unk4C, &figure->unk40);

        if (figure->header.parent != 0) {
            temp2 = figure->header.parent;
            while (temp2 != 0){
                if (!(temp2->header.type & 0x80)) {
                    break;
                }
                temp2 = temp2->header.parent;
            }
            
            if (temp2 != 0) {
                Figure_UpdateMatrices(temp2);
                func_8000CA9C_D69C(&figure->unk68, &temp2->unk68, &figure->unk68);
            }
        }

        sp7C = figure->unk58 - figure->unk98;
        sp78 = figure->unk5C - figure->unk9C;
        sp74 = figure->unk60 - figure->unkA0;
        if (sp7C == 0.0) {
            figure->unk58 += D_800A2790_A3390;
        }

        guLookAtF(&D_80387B30, figure->unk98, figure->unk9C, figure->unkA0, figure->unk58, figure->unk5C, figure->unk60, 0.0f, 1.0f, 0.0f);
        if (figure == (void*)common_camera_game_view) {
            memory_copy(D_80387B30, &D_80389E8C, 0x40U);
        }
        figure->unk52 = atan2f(sp78, sqrtf((sp7C * sp7C) + (sp74 * sp74)));
        figure->unk54 = atan2f((D_80383AB8.unk40A0 < 0.0 ? -D_80383AB8.unk40A0 : D_80383AB8.unk40A0), D_80383AB8.unk4080);
        if (sp74 < 0.0) {
            figure->unk54 = -figure->unk54;    
        }
        figure->unk54 += 0x4000;
        figure->unk56 = 0;

        int figure_idx = (NewFigure*)figure - (NewFigure*)figures_array;

        set_figure_arr_matrix(figure_idx, D_80387B30, FIGURE_MTX_ARRAY_VIEW); // view
        set_figure_arr_matrix(figure_idx, sp80, FIGURE_MTX_ARRAY_PROJ);       // proj

        func_8000CA9C_D69C(D_80387B30, sp80, D_80387B30);
    } else {
        func_80002DA8_39A8(&figure->unk68, &figure->unk4C, &figure->unk40);

        if (figure->header.parent != 0) {
            temp = figure->header.parent;
            while (temp != 0){
                if (!(temp->header.type & 0x80)) {
                    break;
                }
                temp = temp->header.parent;
            }
            
            if (temp != 0) {
                Figure_UpdateMatrices(temp);
                func_8000CA9C_D69C(&figure->unk68, &temp->unk68, &figure->unk68);
            }
        }
        
        sp58 = D_80093360_93F60;

        func_80002A98_3698(&sp58, &figure->unk68, D_80387B30);

        sp7C = D_80383AB8.unk40A8[0] - figure->unk98;
        sp78 = D_80383AB8.unk40A8[1] - figure->unk9C;
        sp74 = D_80383AB8.unk40A8[2] - figure->unkA0;
        if (sp7C == 0.0) {
            D_80383AB8.unk40A8[0] += D_800A2798_A3398;
        }

        guLookAtF(D_80387B30, figure->unk98, figure->unk9C, figure->unkA0, D_80383AB8.unk40A8[0], D_80383AB8.unk40A8[1], D_80383AB8.unk40A8[2], 0.0f, 1.0f, 0.0f);

        figure->unk52 = atan2f(sp78, sqrtf((sp7C * sp7C) + (sp74 * sp74)));

        figure->unk54 = atan2f((D_80387B58 < 0.0 ? -D_80387B58 : D_80387B58), D_80387B38);

        if (sp74 < 0.0) {
            figure->unk54 = -figure->unk54;
        }

        figure->unk54 += 0x4000;
        figure->unk56 = 0;

        func_8000CA9C_D69C(D_80387B30, sp80[0], D_80387B30);
    }
    func_8000CB50_D750(D_80387B30, &D_80387AE8[(NewFigure*)figure - (NewFigure*)figures_array]);
}
