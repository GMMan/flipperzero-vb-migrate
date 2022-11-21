#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/popup.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <gui/modules/text_input.h>
// #include <gui/modules/dialog_ex.h>
#include "gui/modules/file_select.h"
#include <gui/modules/loading.h>

#include <notification/notification.h>
#include <dialogs/dialogs.h>

#include <lib/nfc/nfc_worker.h>

#include "vb_migrate.h"
#include "scenes/vb_migrate_scene.h"

#define VB_MIGRATE_TEMPLATE_NAME "template" NFC_APP_EXTENSION
#define VB_MIGRATE_CAPTURE_FORMAT "%03d%s"

#define VB_MIGRATE_MAX_DEV_NAME_LENGTH (30)

struct VbMigrate {
    Gui* gui;
    Storage* storage;
    DialogsApp* dialogs;
    NotificationApp* notifications;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    Submenu* submenu;
    Popup* popup;
    Widget* widget;
    // DialogEx* dialog_ex;
    FileSelect* file_select;
    TextInput* text_input;
    Loading* loading;
    NfcWorker* worker;
    NfcDevice* nfc_dev;
    char text_store[128];
    uint8_t captured_pwd[4];
    uint8_t captured_uid[7];
    int num_captured;
    int next_id;
    int num_sent;
};

typedef enum {
    VbMigrateViewMenu,
    VbMigrateViewPopup,
    VbMigrateViewWidget,
    VbMigrateViewTextInput,
    // VbMigrateViewDialogEx,
    VbMigrateViewFileSelect,
    VbMigrateViewLoading,
} VbMigrateView;

void vb_migrate_blink_read(VbMigrate* inst);
void vb_migrate_blink_emulate(VbMigrate* inst);
void vb_migrate_blink_stop(VbMigrate* inst);
void vb_migrate_text_store_set(VbMigrate* inst, const char* text, ...);
void vb_migrate_text_store_clear(VbMigrate* inst);
bool vb_migrate_save_nfc(VbMigrate* inst, const char* dev_name, const char* file_name);
bool vb_migrate_load_nfc(VbMigrate* inst, const char* dev_name, const char* file_name);
int vb_migrate_count_captured_mons(VbMigrate* inst, const char* dev_name);
bool vb_migrate_delete(VbMigrate* inst, const char* dev_name, bool whole_vb);
int vb_migrate_get_next_id(VbMigrate* inst, const char* dev_name, int i, bool is_load);
void vb_migrate_show_loading_popup(VbMigrate* inst, bool show);
