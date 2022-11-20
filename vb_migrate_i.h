#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/popup.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>

#include <notification/notification.h>

#include <lib/nfc/nfc_worker.h>

#include "vb_migrate.h"
#include "scenes/vb_migrate_scene.h"

struct VbMigrate {
    Gui* gui;
    NotificationApp* notifications;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    Submenu* submenu;
    Popup* popup;
    Widget* widget;
    NfcWorker* worker;
    NfcDevice* nfc_dev;
    uint8_t captured_pwd[4];
    uint8_t captured_uid[7];
};

typedef enum {
    VbMigrateViewMenu,
    VbMigrateViewPopup,
    VbMigrateViewWidget,
} VbMigrateView;
