#include "../vb_migrate_i.h"

typedef enum {
    SubmenuDevMenuIndexTransferFromApp,
    SubmenuDevMenuIndexTransferToApp,
    SubmenuDevMenuIndexDeleteVb,
} SubmenuDevMenuIndex;

static void vb_migrate_scene_dev_menu_submenu_callback(void* context, uint32_t index) {
    VbMigrate* inst = context;

    view_dispatcher_send_custom_event(inst->view_dispatcher, index);
}

void vb_migrate_scene_dev_menu_on_enter(void* context) {
    VbMigrate* inst = context;
    Submenu* submenu = inst->submenu;

    // Add your menu items
    submenu_add_item(
        submenu,
        "Transfer From App",
        SubmenuDevMenuIndexTransferFromApp,
        vb_migrate_scene_dev_menu_submenu_callback,
        inst);
    submenu_add_item(
        submenu,
        "Transfer To App",
        SubmenuDevMenuIndexTransferToApp,
        vb_migrate_scene_dev_menu_submenu_callback,
        inst);
    submenu_add_item(
        submenu,
        "Delete Vital Bracelet",
        SubmenuDevMenuIndexDeleteVb,
        vb_migrate_scene_dev_menu_submenu_callback,
        inst);

    view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewMenu);
}

bool vb_migrate_scene_dev_menu_on_event(void* context, SceneManagerEvent event) {
    VbMigrate* inst = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuDevMenuIndexTransferFromApp) {
            scene_manager_next_scene(inst->scene_manager, VbMigrateSceneFromApp);
            consumed = true;
        } else if(event.event == SubmenuDevMenuIndexTransferToApp) {
        } else if(event.event == SubmenuDevMenuIndexDeleteVb) {
            scene_manager_next_scene(inst->scene_manager, VbMigrateSceneDelete);
            consumed = true;
        }
    }
    return consumed;
}

void vb_migrate_scene_dev_menu_on_exit(void* context) {
    VbMigrate* inst = context;

    submenu_reset(inst->submenu);
}
