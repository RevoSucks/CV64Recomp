#include "patches.h"
#include "misc_funcs.h"
#include "graphics.h"

#include "object_funcs.h"

RECOMP_EXPORT Object *get_object_from_ID(s16 ID) {
    for (int i = 0; i < OBJECT_ARRAY_MAX; i++) {
        s16 compareID = objects_array[i].header.ID;

        // unmask the bits.
        compareID &= ~(OBJ_FLAG_ENABLE_COLLISION);
        compareID &= ~(OBJ_FLAG_MAP_OVERLAY);
        compareID &= ~(OBJ_FLAG_DESTROY);
        compareID &= ~(OBJ_FLAG_MOVE_ALONGSIDE_COLLISION);

        ID &= ~(OBJ_FLAG_ENABLE_COLLISION);
        ID &= ~(OBJ_FLAG_MAP_OVERLAY);
        ID &= ~(OBJ_FLAG_DESTROY);
        ID &= ~(OBJ_FLAG_MOVE_ALONGSIDE_COLLISION);

        if (objects_array[i].header.ID == ID) {
            return &objects_array[i];
        }
    }
    recomp_printf("[get_object_from_ID] WARNING: Object not found from ID 0x%08X\n", ID);
    return NULL;
}
