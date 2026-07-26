#include "patches.h"
#include "misc_funcs.h"
#include "graphics.h"

#include "gfx/figure.h"
#include "gfx/camera.h"
#include "gfx/model.h"

#include "game/object.h"

#include "figure_funcs.h"
#include "object_funcs.h"

RECOMP_EXPORT Figure *get_root_figure(void *arg0) {
    struct NewFigure *ptr = (struct NewFigure *)arg0;

    // seek to the master struct value. This ascends the heirarchy until we found the 'master' struct.
    while (ptr->header.parent) {
        ptr = (NewFigure *)ptr->header.parent;
    }

    return ptr;
}

RECOMP_EXPORT s16 get_id_from_figure(void *arg0, int figure_idx) {
    struct NewFigure *ptr = get_root_figure(arg0);
    s16 ID = 0; // no match

    int i;

    // search for the matching object.
    for(i = 0; i < OBJECT_ARRAY_MAX; i++) {
        if (objects_array[i].figures[0] == (u32)ptr) {
            ID = objects_array[i].header.ID;
            break;
        }
    }

    // remove bits
    if (ID == -1 || ID == 0) {
        return ID;
    }

    ID &= ~(OBJ_FLAG_ENABLE_COLLISION);
    ID &= ~(OBJ_FLAG_MAP_OVERLAY);
    ID &= ~(OBJ_FLAG_DESTROY);
    ID &= ~(OBJ_FLAG_MOVE_ALONGSIDE_COLLISION);
    return ID;
}

RECOMP_EXPORT u32 get_tag_from_figure(void *arg0, int figure_idx) {
    struct NewFigure *ptr = get_root_figure(arg0);
    s16 ID = 0; // no match

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
        return ((u32)ID << 16) | figure_idx;
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

/**
 * Check if the given figure vs the hud pointer (if it exists) is part of the HUD, and
 * if so, return if it should be left aligned or right-aligned.
 */
RECOMP_EXPORT enum HudObjectType check_figure_for_hud(void *arg0, Object *hud) {
    struct NewFigure *ptr = (struct NewFigure *)arg0;

    // hardcoded ptr check
    if ((u32)get_root_figure(ptr) == (u32)common_camera_8009B440->next) {
        //recomp_printf("[check_figure_for_hud] camera mode found\n");
        return HUD_OBJECT_CAMERA_MODE;
    }

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
