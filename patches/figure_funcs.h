#ifndef _FIGURE_FUNCS_RECOMP_H_
#define _FIGURE_FUNCS_RECOMP_H_

#include "gfx/figure.h"
#include "gfx/camera.h"

#include "game/object.h"

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
    u8 filler40[0xC];
    s16 unk4C;
    s16 unk4E;
    s16 unk50;
    u8 filler52[0x56];
} NewFigure; // Size = 0xA8

enum HudObjectType {
    HUD_OBJECT_BAR,       // needs to be left aligned
    HUD_OBJECT_SUBWEAPON, // needs to be right aligned
    HUD_OBJECT_CAMERA_MODE, // left aligned bottom left text
    HUD_OBJECT_NEITHER, // return this if it's not the HUD
};

extern Camera* common_camera_8009B440;

RECOMP_EXPORT Figure *get_root_figure(void *arg0);
RECOMP_EXPORT s16 get_id_from_figure(void *arg0, int figure_idx);
RECOMP_EXPORT u32 get_tag_from_figure(void *arg0, int figure_idx);
RECOMP_EXPORT Object *get_hud_object();
RECOMP_EXPORT int is_pause_menu_spawned();
RECOMP_EXPORT enum HudObjectType check_figure_for_hud(void *arg0, Object *hud);

#endif // _FIGURE_FUNCS_RECOMP_H_
