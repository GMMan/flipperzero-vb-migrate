#include "../vb_migrate_i.h"

#define TAG "vb_migrate_scene_load"

void vb_migrate_scene_load_on_enter(void* context) {
    VbMigrate* inst = context;

    // Perform your setup here
    view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewLoading);
    view_dispatcher_send_custom_event(inst->view_dispatcher, 0);
}

bool vb_migrate_scene_load_on_event(void* context, SceneManagerEvent event) {
    VbMigrate* inst = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(vb_migrate_load_nfc(inst, inst->text_store, VB_MIGRATE_TEMPLATE_NAME)) {
            scene_manager_next_scene(inst->scene_manager, VbMigrateSceneInfo);
            consumed = true;
        } else {
            consumed = scene_manager_previous_scene(inst->scene_manager);
        }
    }
    return consumed;
}

void vb_migrate_scene_load_on_exit(void* context) {
    UNUSED(context);
}
