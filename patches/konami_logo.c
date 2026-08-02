#include "patches.h"
#include "misc_funcs.h"
#include "graphics.h"

#include "objects/engine/konami_kcek_logos.h"

#include "gfx/model.h"
#include "game/object.h"
#include "system_work.h"

#include "figure_funcs.h"

extern cv64_ovl_konamilogo_func_t cv64_ovl_konamilogo_funcs[];

RECOMP_EXPORT void extend_vtx_list(Vtx_t *vtx, int count) {
    for (int i = 0; i < count; i++) {
        // this vertex is on the left edge. Extend it left infinitely.
        if (vtx[i].ob[0] == -160) {
            vtx[i].ob[0] = -32768;
        } else if (vtx[i].ob[0] == 160) { // same for right edge
            vtx[i].ob[0] = 32767;
        }
    }
}

RECOMP_PATCH void cv64_ovl_konamilogo_init(cv64_ovl_konamilogo_t* self) {
    Model* model = (*Model_createAndSetChild)(FIG_TYPE_HUD_ELEMENT, common_camera_HUD);

    self->model                  = model;
    model->dlist                 = (u32)&KONAMI_LOGO_DL;
    model->assets_file           = NI_ASSETS_KONAMI_AND_KCEK_LOGOS;
    model->size.x                = 0.9975f;
    model->size.y                = 1.005f;
    sys.background_color.integer = RGBA(0, 0, 0, 255);
    BITS_SET(model->flags, FIG_FLAG_APPLY_PRIMITIVE_COLOR);
    model->primitive_color.integer = RGBA(255, 255, 255, 0);

    /**
     * @recomp Patch the vertex lists to extend the rects left and right due to the color
     * being blended twice, resulting in widescreen not looking correct. We will extend
     * the "front" later vertices so it covers the entire screen so the back isn't
     * exposed.
     */
    u8 *ptr = sys.Nisitenma_Ichigo_loaded_files_ptr[model->assets_file];
    
    extend_vtx_list((Vtx_t *)(ptr + 0xBE00), 32);
    extend_vtx_list((Vtx_t *)(ptr + 0xC000), 29);

    GO_TO_NEXT_FUNC_NOW(self, cv64_ovl_konamilogo_funcs);
}

RECOMP_PATCH void cv64_ovl_konamilogo_kcek_fade_in(cv64_ovl_konamilogo_t* self) {
    Model* model = self->model;

    model->size.x = 0.995f;
    model->dlist  = (u32)&KCEK_LOGO_DL;
    if (model->primitive_color.a < 252) {
        model->primitive_color.a += 3;
    } else {
        model->primitive_color.a = 255;
        (*object_curLevel_goToNextFuncAndClearTimer)(
            self->header.current_function, &self->header.function_info_ID
        );
    }

    /**
     * @recomp Same for the KCEK logo.
     */
    u8 *ptr = sys.Nisitenma_Ichigo_loaded_files_ptr[model->assets_file];

    extend_vtx_list((Vtx_t *)(ptr + 0x15340), 31);
    extend_vtx_list((Vtx_t *)(ptr + 0x15530), 32);
    extend_vtx_list((Vtx_t *)(ptr + 0x15730), 32);
    extend_vtx_list((Vtx_t *)(ptr + 0x15930), 7);

    cv64_ovl_konamilogo_check_btn_press(self);
}
