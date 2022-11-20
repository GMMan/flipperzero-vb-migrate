#include "../vb_migrate_i.h"

typedef enum {
    SubmenuMainIndexRegister,
    SubmenuMainIndexSelect,
    SubmenuMainIndexDelete,
} SubmenuMainIndex;

void vb_migrate_scene_main_menu_submenu_callback(void* context, uint32_t index) {
    VbMigrate* inst = context;

    view_dispatcher_send_custom_event(inst->view_dispatcher, index);
}

void vb_migrate_scene_main_menu_on_enter(void* context) {
    VbMigrate* inst = context;
    Submenu* submenu = inst->submenu;

    submenu_add_item(
        submenu,
        "Register Vital Bracelet",
        SubmenuMainIndexRegister,
        vb_migrate_scene_main_menu_submenu_callback,
        inst);
    submenu_add_item(
        submenu,
        "Select Vital Bracelet",
        SubmenuMainIndexSelect,
        vb_migrate_scene_main_menu_submenu_callback,
        inst);
    submenu_add_item(
        submenu,
        "Delete Vital Bracelet",
        SubmenuMainIndexDelete,
        vb_migrate_scene_main_menu_submenu_callback,
        inst);

    view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewMenu);
}

bool vb_migrate_scene_main_menu_on_event(void* context, SceneManagerEvent event) {
    VbMigrate* inst = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuMainIndexRegister) {
            scene_manager_next_scene(inst->scene_manager, VbMigrateSceneRegister);
            consumed = true;
        } else if(event.event == SubmenuMainIndexSelect) {
        } else if(event.event == SubmenuMainIndexDelete) {
        }
    }
    return consumed;
}

void vb_migrate_scene_main_menu_on_exit(void* context) {
    VbMigrate* inst = context;

    submenu_reset(inst->submenu);
}
