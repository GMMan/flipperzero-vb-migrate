#include "../vb_migrate_i.h"
#include "vb_migrate_icons.h"

typedef enum {
    SaveSuccessEventPopup,
} SaveSuccessEvent;

void vb_migrate_scene_save_success_popup_callback(void* context) {
    VbMigrate* inst = context;

    view_dispatcher_send_custom_event(inst->view_dispatcher, SaveSuccessEventPopup);
}

void vb_migrate_scene_save_success_on_enter(void* context) {
    VbMigrate* inst = context;

    // Setup view
    Popup* popup = inst->popup;
    popup_set_icon(popup, 32, 5, &I_DolphinNice_96x59);
    popup_set_header(popup, "Saved!", 13, 22, AlignLeft, AlignBottom);
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, inst);
    popup_set_callback(popup, vb_migrate_scene_save_success_popup_callback);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewPopup);
}

bool vb_migrate_scene_save_success_on_event(void* context, SceneManagerEvent event) {
    VbMigrate* inst = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SaveSuccessEventPopup) {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                inst->scene_manager, VbMigrateSceneMainMenu);
        }
    }
    return consumed;
}

void vb_migrate_scene_save_success_on_exit(void* context) {
    VbMigrate* inst = context;

    // Clear view
    popup_reset(inst->popup);
}
