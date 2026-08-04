#include "patches.h"
#include "misc_funcs.h"
#include "graphics.h"

#include "mtx.h"

// cant use the game header as it has the wrong atan2f type.
//#include "gfx/camera.h"

struct ReplacedSysWork {
    char filler0[0x26440];
    u32 cutscene_ID;
};

// hack because we cannot include system_work.h without using the broken atan2f definition.
extern struct ReplacedSysWork sys;

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

typedef struct Vec3f {
    f32 x, y, z;
} Vec3f;

typedef struct Angle {
    s16 pitch, yaw, roll;
} Angle;

typedef struct Vec3 {
    s16 x, y, z;
} Vec3;

typedef f32 Mat4f[4][4];

typedef struct Camera {
    s16 type;
    u16 flags;
    struct Camera* prev;
    struct Camera* sibling;
    struct Camera* next;
    struct Camera* parent;
    u8 field1_0x14;
    u8 field2_0x15;
    u8 field3_0x16;
    u8 field4_0x17;
    u8 field5_0x18;
    u8 field6_0x19;
    u8 field7_0x1a;
    u8 field8_0x1b;
    u8 field9_0x1c;
    u8 field10_0x1d;
    u8 field11_0x1e;
    u8 field12_0x1f;
    u8 field13_0x20;
    u8 field14_0x21;
    u8 field15_0x22;
    u8 field16_0x23;
    u16 perspNorm;
    u8 field18_0x26;
    u8 field19_0x27;
    u8 field20_0x28;
    u8 field21_0x29;
    u8 field22_0x2a;
    u8 field23_0x2b;
    u8 field24_0x2c;
    u8 field25_0x2d;
    u8 field26_0x2e;
    u8 field27_0x2f;
    void* screen_params;
    void* projection_matrix_params;
    u8 field30_0x38;
    u8 field31_0x39;
    u8 field32_0x3a;
    u8 field33_0x3b;
    Gfx* clip_ratio_dl;
    Vec3f position;
    Vec3 field36_0x4c;
    Angle angle;
    Vec3f look_at_direction;
    u8 field39_0x64;
    u8 field40_0x65;
    u8 field41_0x66;
    u8 field42_0x67;
    Mat4f matrix;
} Camera;

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

static int times = 0;
static int init = 0;

#define CUT_THRESHOLD_POS 50
#define CUT_THRESHOLD_ANGLE 350

Vec3f last_camera_pos = { 0.0f, 0.0f, 0.0f };
Angle last_camera_angle = { 0, 0, 0 };

extern int camera_interpolation_disabled;

// -------------------------------------------
// shamelessly stolen from banjo-kazooie.
#define TUPLE_DIFF_COPY(dst, vec1, vec2) { \
    dst[0] = vec1[0] - vec2[0]; \
    dst[1] = vec1[1] - vec2[1]; \
    dst[2] = vec1[2] - vec2[2]; \
}

#define LENGTH_SQ_VEC3F(v) (v[0]*v[0] + v[1]*v[1] + v[2]*v[2])
#define LENGTH_VEC3F(v) sqrtf(LENGTH_SQ_VEC3F(v))

RECOMP_EXPORT f32 ml_vec3f_distance(f32 vec1[3], f32 vec2[3]) {
    f32 diff[3];
    TUPLE_DIFF_COPY(diff, vec1, vec2)
    return LENGTH_VEC3F(diff);
}
// -------------------------------------------

f32 get_camera_cut_threshold_pos() {
    switch(sys.cutscene_ID) {
        default: // cutscene ID not supported. use the default threshold.
            return CUT_THRESHOLD_POS;
    }
}

f32 get_camera_cut_threshold_angle() {
    switch(sys.cutscene_ID) {
        default: // cutscene ID not supported. use the default threshold.
            return CUT_THRESHOLD_ANGLE;
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

    // @recomp check for camera cuts. Are we in a cutscene?
    if (figure == (void*)common_camera_game_view && common_camera_game_view && sys.cutscene_ID) {
        Camera *c = common_camera_game_view;

        f32 pos_threshold = get_camera_cut_threshold_pos();
        f32 angle_threshold = get_camera_cut_threshold_angle();

        if (init == 0) {
            last_camera_pos.x = c->position.x;
            last_camera_pos.y = c->position.y;
            last_camera_pos.z = c->position.z;
            last_camera_angle.pitch = c->angle.pitch;
            last_camera_angle.yaw = c->angle.yaw;
            last_camera_angle.roll = c->angle.roll;
            init = 1;
        } else {
            f32 dist = ml_vec3f_distance((f32*)&last_camera_pos, (f32*)&c->position);
            f32 dp = (s16)(c->angle.pitch - last_camera_angle.pitch);
            f32 dy = (s16)(c->angle.yaw - last_camera_angle.yaw);
            f32 dr = (s16)(c->angle.roll - last_camera_angle.roll);
            f32 da = sqrtf(dp * dp + dy * dy + dr * dr);

            if (dist > pos_threshold) {
                camera_interpolation_disabled = 1;
                recomp_printf("[func_801299A4] %d Camera cut detected for cutscene ID %d pos %.6f\n", times++, sys.cutscene_ID, dist);
            } else if (da > angle_threshold) {
                camera_interpolation_disabled = 1;
                recomp_printf("[func_801299A4] %d Camera cut detected for cutscene ID %d angle %.6f\n", times++, sys.cutscene_ID, da);
            } else {
                camera_interpolation_disabled = 0;
            }

            last_camera_pos.x = c->position.x;
            last_camera_pos.y = c->position.y;
            last_camera_pos.z = c->position.z;
            last_camera_angle.pitch = c->angle.pitch;
            last_camera_angle.yaw = c->angle.yaw;
            last_camera_angle.roll = c->angle.roll;
        }
    }
}
