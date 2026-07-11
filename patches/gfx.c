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

extern Vp D_80092F58_93B58;

RECOMP_EXPORT u32 get_tag_from_figure(void *arg0, int figure_idx) {
    struct NewFigure *ptr = (struct NewFigure *)arg0;
    s16 ID = 0; // no match

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
    if (ID != 0) {
        // for -1 IDs, we mask their object ID on top of it, since this is a geometry object.
        if (ID != -1) {
            // unmask the object bits.
            ID &= ~(OBJ_FLAG_ENABLE_COLLISION);
            ID &= ~(OBJ_FLAG_MAP_OVERLAY);
            ID &= ~(OBJ_FLAG_DESTROY);
            ID &= ~(OBJ_FLAG_MOVE_ALONGSIDE_COLLISION);

            return ((u32)ID << 16) | i; // use object IDX for objects.
        } else {
            // unmask the object bits.
            ID &= ~(OBJ_FLAG_ENABLE_COLLISION);
            ID &= ~(OBJ_FLAG_MAP_OVERLAY);
            ID &= ~(OBJ_FLAG_DESTROY);
            ID &= ~(OBJ_FLAG_MOVE_ALONGSIDE_COLLISION);

            return ((u32)ID << 16) | (figure_idx | 0x00008000); // this is geometry. Use figure IDX instead.
        }
    }
    return ID;
}

RECOMP_EXPORT Object *get_hud_object() {
    for(int i = 0; i < OBJECT_ARRAY_MAX; i++) {
        if (objects_array[i].header.ID == 0x0129) {
            return &objects_array[i];
        }
    }
    return NULL;
}

RECOMP_EXPORT int is_pause_menu_spawned() {
    for(int i = 0; i < OBJECT_ARRAY_MAX; i++) {
        if ((objects_array[i].header.ID & 0x0FFF) == 0x134) {
            return 1;
        }
    }
    return 0;
}

enum HudObjectType {
    HUD_OBJECT_BAR,       // needs to be left aligned
    HUD_OBJECT_SUBWEAPON, // needs to be right aligned
    HUD_OBJECT_NEITHER, // return this if it's not the HUD
};

RECOMP_EXPORT enum HudObjectType check_figure_for_hud(void *arg0, Object *hud) {
    struct NewFigure *ptr = (struct NewFigure *)arg0;

    // if the HUD isnt even loaded, no need to even check (nor did we receive a valid Figure pointer)
    if (hud == NULL || ptr == NULL) {
        return HUD_OBJECT_NEITHER;
    }

    if ((hud->figures[0] == (Figure *)ptr) || (hud->figures[4] == (Figure *)ptr)
     || (hud->figures[5] == (Figure *)ptr) || (hud->figures[6] == (Figure *)ptr)
     || (hud->figures[7] == (Figure *)ptr) || (hud->figures[8] == (Figure *)ptr)
     || (hud->figures[9] == (Figure *)ptr)) { 
        return HUD_OBJECT_BAR; // clock and HP bar (left align)
    } else if ((hud->figures[1] == (Figure *)ptr) || (hud->figures[2] == (Figure *)ptr)
            || (hud->figures[3] == (Figure *)ptr) || (hud->figures[12] == (Figure *)ptr)
            || (hud->figures[13] == (Figure *)ptr) || (hud->figures[14] == (Figure *)ptr)
            || (hud->figures[10] == (Figure *)ptr) || (hud->figures[11] == (Figure *)ptr)) {
        return HUD_OBJECT_SUBWEAPON; // boss bar, status/gold/good, subweapon box (right align)
    }

    return HUD_OBJECT_NEITHER;
}

// geometry?
RECOMP_PATCH void func_80005684_6284(NewFigure* arg0) {
    int i;

    int figure_idx = (NewFigure*)arg0 - (NewFigure*)figures_array;

    u32 tag = get_tag_from_figure(arg0, figure_idx);

    gSPViewport(gDisplayListHead++, arg0->unk30);

    if (!(arg0->header.type & 0x0100)) {
        gSPPerspNormalize(gDisplayListHead++, arg0->unk24);
    }

    gSPMatrix(gDisplayListHead++, &D_80387AE8[figure_idx], G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

    for (i = 0; i < 5; i++) {
        *(gDisplayListHead++) = arg0->u.unk3C_gfx[i];
    }
}

extern Mtx* D_80387AE8;

extern void func_80005AD8_66D8(NewFigure *, u32);

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
    if ((is_pause_menu_spawned() == 0) && (hud_object_type == HUD_OBJECT_BAR || hud_object_type == HUD_OBJECT_SUBWEAPON)) {
#define ALIGN_UI_LEFT  0
#define ALIGN_UI_RIGHT 1
        int alignType = (hud_object_type == HUD_OBJECT_BAR) ? ALIGN_UI_LEFT : ALIGN_UI_RIGHT;

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

    gEXMatrixGroupDecomposedNormal(gDisplayListHead++, tag, G_MTX_PUSH, G_MTX_MODELVIEW, G_EX_EDIT_NONE);
    func_80005AD8_66D8(arg0, (u32)arg0->unk34);
    gEXPopMatrixGroup(gDisplayListHead++, G_MTX_MODELVIEW);

    if (pop_viewport) {
        gEXPopScissor(gDisplayListHead++);
        gEXPopViewport(gDisplayListHead++);
    }
}
