#include "patches.h"
#include "misc_funcs.h"

#include "cv64.h"
#include "game/system_work.h"
#include "game/gamestate.h"
#include "game/object.h"

// gfx.c

#define gEXMatrixGroupDecomposedNormal(cmd, id, push, proj, edit) \
    gEXMatrixGroupDecomposed(cmd, id, push, proj, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_INTERPOLATE, G_EX_ORDER_LINEAR, edit, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO)

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

// Generic figure struct
// Generic figure struct
typedef struct NewFigure {
    FigureHeader header;
    u8 field_0x14[0x24 - 0x14];
    /* 0x24 */ u16 unk24;
    /* 0x26 */ u16 unk26;
    /* 0x28 */ u32 *unk28;
    char filler2C[0x4];
    /* 0x30 */ void *unk30;
    void *unk34;
    void *unk38;
    union Unk3CUnion {
        Gfx *unk3C_gfx;
        u32 unk3C_u32;
    } u;
    u8 pad40[0x68];
} NewFigure; // Size = 0xA8

#define FIGURE_ID_MASK 0x8000

struct UnkStruct8000C800_Input {
    char filler0[0x10];
    void *unk10;
}; // unknown size

RECOMP_EXPORT u32 get_tag_from_figure(void *arg0) {
    struct NewFigure *ptr = (struct NewFigure *)arg0;
    s16 ID = -1;

    // seek to the master struct value. This ascends the heirarchy until we found the 'master' struct.
    while (ptr->header.parent) {
        ptr = (NewFigure *)ptr->header.parent;
    }

    int i;

    // search for the matching object.
    for(i = 0; i < OBJECT_ARRAY_MAX; i++) {
        if (objects_array[i].figures[0] == (u32)ptr) {
            ID = objects_array[i].header.ID;
            break;
        }
    }

    // the ID is in use. Try tagging it.
    if (ID != -1 && ID != 0) {
        // unmask the object bits.
        ID &= ~(OBJ_FLAG_ENABLE_COLLISION);
        ID &= ~(OBJ_FLAG_MAP_OVERLAY);
        ID &= ~(OBJ_FLAG_DESTROY);
        ID &= ~(OBJ_FLAG_MOVE_ALONGSIDE_COLLISION);

        return (ID << 16) | i;
    }
    return 0xFFFFFFFF;
}

// geometry?
RECOMP_PATCH void func_80005684_6284(NewFigure* arg0) {
    int i;
    
    gSPViewport(gDisplayListHead++, arg0->unk30);

    if (!(arg0->header.type & 0x0100)) {
        gSPPerspNormalize(gDisplayListHead++, arg0->unk24);
    }

    int figure_idx = (NewFigure*)arg0 - (NewFigure*)figures_array;

    gSPMatrix(gDisplayListHead++, &D_80387AE8[figure_idx], G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

    for (i = 0; i < 5; i++) {
        *(gDisplayListHead++) = arg0->u.unk3C_gfx[i];
    }
}

extern Mtx* D_80387AE8;

extern void func_80005AD8_66D8(NewFigure *, u32);

static u32 global_tag = 0xFFFFFFFF;

RECOMP_PATCH void func_80006194_6D94(NewFigure * arg0) {
    int figure_idx;
    if (arg0->unk38 != NULL) {
        figure_idx = (NewFigure*)arg0->header.prev - (NewFigure*)figures_array;

        gSPMatrix(gDisplayListHead++, &D_80387AE8[figure_idx], G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

        u32 tag = get_tag_from_figure(arg0);

        recomp_printf("[func_80006194_6D94] tag 1 0x%08X\n", tag);
        gEXMatrixGroupDecomposedNormal(gDisplayListHead++, tag, G_MTX_PUSH, G_MTX_MODELVIEW, G_EX_EDIT_NONE);
        func_80005AD8_66D8(arg0, (u32)arg0->unk38);
        gEXPopMatrixGroup(gDisplayListHead++, G_MTX_MODELVIEW);
    }

    figure_idx = (NewFigure*)arg0 - (NewFigure*)figures_array;
    u32 tag = get_tag_from_figure(arg0);

    gSPMatrix(gDisplayListHead++, &D_80387AE8[figure_idx], G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

    if (arg0->unk30 != NULL) {
        u32 tag = get_tag_from_figure(arg0);
        
        recomp_printf("[func_80006194_6D94] tag 2 0x%08X\n", tag);
        gEXMatrixGroupDecomposedNormal(gDisplayListHead++, tag, G_MTX_PUSH, G_MTX_MODELVIEW, G_EX_EDIT_NONE);
        func_80005AD8_66D8(arg0, (u32)arg0->unk30);
        gEXPopMatrixGroup(gDisplayListHead++, G_MTX_MODELVIEW);
    }

    tag = get_tag_from_figure(arg0);
    
    recomp_printf("[func_80006194_6D94] tag 3 0x%08X\n", tag);
    gEXMatrixGroupDecomposedNormal(gDisplayListHead++, tag, G_MTX_PUSH, G_MTX_MODELVIEW, G_EX_EDIT_NONE);
    func_80005AD8_66D8(arg0, (u32)arg0->unk34);
    gEXPopMatrixGroup(gDisplayListHead++, G_MTX_MODELVIEW);
}
