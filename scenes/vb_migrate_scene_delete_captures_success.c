#include "../vb_migrate_i.h"
#include "vb_migrate_icons.h"

static void vb_migrate_scene_delete_captures_success_popup_callback(void* context) {
    VbMigrate* inst = context;
    view_dispatcher_send_custom_event(inst->view_dispatcher, 0);
}

void vb_migrate_scene_delete_captures_success_on_enter(void* context) {
    VbMigrate* inst = context;

    // Perform your setup here
    Popup* popup = inst->popup;
    popup_set_icon(popup, 0, 2, &I_DolphinMafia_115x62);
    popup_set_header(popup, "Cleared", 83, 19, AlignLeft, AlignBottom);
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, inst);
    popup_set_callback(popup, vb_migrate_scene_delete_captures_success_popup_callback);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewPopup);
}

bool vb_migrate_scene_delete_captures_success_on_event(void* context, SceneManagerEvent event) {
    VbMigrate* inst = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = scene_manager_search_and_switch_to_previous_scene(
            inst->scene_manager, VbMigrateSceneDevMenu);
    }
    return consumed;
}

void vb_migrate_scene_delete_captures_success_on_exit(void* context) {
    VbMigrate* inst = context;

    // Perform your cleanup here
    popup_reset(inst->popup);
}
