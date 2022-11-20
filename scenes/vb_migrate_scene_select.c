#include "../vb_migrate_i.h"

void vb_migrate_scene_select_file_select_callback(bool result, void* context) {
    VbMigrate* inst = context;

    view_dispatcher_send_custom_event(inst->view_dispatcher, result);
}

void vb_migrate_scene_select_on_enter(void* context) {
    VbMigrate* inst = context;

    // Perform your setup here
    file_select_set_callback(
        inst->file_select, vb_migrate_scene_select_file_select_callback, inst);
    file_select_set_filter(inst->file_select, VB_MIGRATE_FOLDER, "*");
    file_select_set_result_buffer(inst->file_select, inst->text_store, sizeof(inst->text_store));
    file_select_init(inst->file_select);

    view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewFileSelect);
}

bool vb_migrate_scene_select_on_event(void* context, SceneManagerEvent event) {
    VbMigrate* inst = context;
    bool consumed = false;
    UNUSED(inst);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event) {
            // File selected
            scene_manager_next_scene(inst->scene_manager, VbMigrateSceneLoad);
            consumed = true;
        }
    }
    return consumed;
}

void vb_migrate_scene_select_on_exit(void* context) {
    UNUSED(context);
}
