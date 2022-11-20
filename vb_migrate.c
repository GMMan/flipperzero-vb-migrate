#include <notification/notification_messages.h>

#include "vb_migrate_i.h"

bool vb_migrate_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    VbMigrate* inst = context;
    return scene_manager_handle_custom_event(inst->scene_manager, event);
}

bool vb_migrate_back_event_callback(void* context) {
    furi_assert(context);
    VbMigrate* inst = context;
    return scene_manager_handle_back_event(inst->scene_manager);
}

void vb_migrate_blink_read(VbMigrate* inst) {
    notification_message(inst->notifications, &sequence_blink_start_cyan);
}

void vb_migrate_blink_emulate(VbMigrate* inst) {
    notification_message(inst->notifications, &sequence_blink_start_magenta);
}

void vb_migrate_blink_stop(VbMigrate* inst) {
    notification_message_block(inst->notifications, &sequence_blink_stop);
}

VbMigrate* vb_migrate_alloc() {
    VbMigrate* inst = malloc(sizeof(VbMigrate));

    inst->view_dispatcher = view_dispatcher_alloc();
    inst->scene_manager = scene_manager_alloc(&vb_migrate_scene_handlers, inst);
    view_dispatcher_enable_queue(inst->view_dispatcher);
    view_dispatcher_set_event_callback_context(inst->view_dispatcher, inst);
    view_dispatcher_set_custom_event_callback(
        inst->view_dispatcher, vb_migrate_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        inst->view_dispatcher, vb_migrate_back_event_callback);

    // GUI
    inst->gui = furi_record_open(RECORD_GUI);

    // Notifications service
    inst->notifications = furi_record_open(RECORD_NOTIFICATION);

    // NFC
    inst->worker = nfc_worker_alloc();
    inst->nfc_dev = nfc_device_alloc();

    // Submenu
    inst->submenu = submenu_alloc();
    view_dispatcher_add_view(
        inst->view_dispatcher, VbMigrateViewMenu, submenu_get_view(inst->submenu));

    // Popup
    inst->popup = popup_alloc();
    view_dispatcher_add_view(
        inst->view_dispatcher, VbMigrateViewPopup, popup_get_view(inst->popup));

    // Widget
    inst->widget = widget_alloc();
    view_dispatcher_add_view(
        inst->view_dispatcher, VbMigrateViewWidget, widget_get_view(inst->widget));

    return inst;
}

void vb_migrate_free(VbMigrate* inst) {
    // Widget
    view_dispatcher_remove_view(inst->view_dispatcher, VbMigrateViewWidget);
    widget_free(inst->widget);

    // Popup
    view_dispatcher_remove_view(inst->view_dispatcher, VbMigrateViewPopup);
    popup_free(inst->popup);

    // Submenu
    view_dispatcher_remove_view(inst->view_dispatcher, VbMigrateViewMenu);
    submenu_free(inst->submenu);

    // NFC
    nfc_device_free(inst->nfc_dev);
    nfc_worker_free(inst->worker);

    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);

    view_dispatcher_free(inst->view_dispatcher);
    scene_manager_free(inst->scene_manager);

    free(inst);
}

int32_t vb_migrate_app(void* p) {
    UNUSED(p);

    VbMigrate* inst = vb_migrate_alloc();
    view_dispatcher_attach_to_gui(inst->view_dispatcher, inst->gui, ViewDispatcherTypeFullscreen);
    scene_manager_next_scene(inst->scene_manager, VbMigrateSceneMainMenu);

    view_dispatcher_run(inst->view_dispatcher);

    vb_migrate_free(inst);
    return 0;
}
