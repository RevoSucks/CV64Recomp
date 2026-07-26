#include "patches.h"
#include "misc_funcs.h"
#include "graphics.h"

#include "game/animation.h"

#include "figure_funcs.h"
#include "game/objects/engine/object_0003.h"

extern void object_destroyChildrenAndModelInfo(ObjectHeader* self);

static int timer = 0;

/**
 * Creates a new object and makes it so that
 * the new object is executed after all the `parent`'s children
 * are executed beforehand.
 */
RECOMP_PATCH ObjectHeader* object_createAndSetChild(ObjectHeader* parent, ObjectID ID) {
    // Allocate the object in the objects array
    ObjectHeader* new_object = object_allocate(ID);
    ObjectHeader* var_v0;
    ObjectHeader* var_v1;

    if (new_object != NULL) {
        // Grab the part of the "ID" field that contains the actual ID of the object
        BITS_ASSIGN_MASK(ID, 0x7FF);

        if (objects_file_info[ID - 1] != NULL && func_8000EE18(ptr_Object_0003, new_object) == -1) {
            return NULL;
        }

        new_object->function_info_ID = -1;

        if (parent != NULL) {
            new_object->parent = parent;

            if (parent->child != NULL) {
                var_v0 = parent->child->next;
                var_v1 = parent->child;

                // Traverse all the parent's child "next" pointers until the
                // last one is reached. Then put the new one in there.
                for (; var_v0 != NULL; var_v0 = var_v0->next) {
                    var_v1 = var_v0;
                }

                var_v1->next = new_object;
            } else {
                parent->child = new_object;
            }
        }

        new_object->destroy = (void*)&object_destroyChildrenAndModelInfo;
        objects_number_of_instances_per_object[ID - 1]++;
    }

    return new_object;
}
