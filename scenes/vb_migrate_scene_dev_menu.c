// SPDX-License-Identifier: GPL-3.0-or-later
//
// VB Lab Migration Assistant for Flipper Zero
// Copyright (C) 2022  cyanic
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "../vb_migrate_i.h"

typedef enum {
    SubmenuDevMenuIndexTransferFromApp,
    SubmenuDevMenuIndexTransferToApp,
    SubmenuDevMenuIndexClearCaptures,
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
        "Transfer App > Flipper",
        SubmenuDevMenuIndexTransferFromApp,
        vb_migrate_scene_dev_menu_submenu_callback,
        inst);
    if(inst->num_captured != 0) {
        submenu_add_item(
            submenu,
            "Transfer Flipper > App",
            SubmenuDevMenuIndexTransferToApp,
            vb_migrate_scene_dev_menu_submenu_callback,
            inst);
        submenu_add_item(
            submenu,
            "Clear Captures",
            SubmenuDevMenuIndexClearCaptures,
            vb_migrate_scene_dev_menu_submenu_callback,
            inst);
    }
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
            scene_manager_next_scene(inst->scene_manager, VbMigrateSceneToApp);
            consumed = true;
        } else if(event.event == SubmenuDevMenuIndexClearCaptures) {
            scene_manager_next_scene(inst->scene_manager, VbMigrateSceneDeleteCaptures);
            consumed = true;
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
