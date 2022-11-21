#include <notification/notification_messages.h>

#include "../vb_migrate_i.h"
#include "../vb_tag.h"

#define TAG "vb_migrate_scene_to_app"

typedef enum {
    ToAppStateInitial,
    ToAppStateInstructions,
    ToAppStateEmulateReady,
    ToAppStateEmulateTransferToApp,
    ToAppStateLoadError,
    ToAppStateComplete,
} ToAppState;

typedef enum {
    ToAppEventTypeWidgetLeft,
    ToAppEventTypeWidgetRight,
    ToAppEventTypeEmulateStart,
    ToAppEventTypeCaptureLoadError,
    ToAppEventTypeTagWrite,
} ToAppEventType;

static void
    vb_migrate_scene_to_app_widget_callback(GuiButtonType result, InputType type, void* context) {
    VbMigrate* inst = context;

    if(type == InputTypeShort) {
        if(result == GuiButtonTypeLeft)
            view_dispatcher_send_custom_event(inst->view_dispatcher, ToAppEventTypeWidgetLeft);
        else if(result == GuiButtonTypeRight)
            view_dispatcher_send_custom_event(inst->view_dispatcher, ToAppEventTypeWidgetRight);
    }
}

static bool vb_migrate_scene_to_app_worker_callback(NfcWorkerEvent event, void* context) {
    VbMigrate* inst = context;
    bool result = false;

    if(event == NfcWorkerEventSuccess) {
        view_dispatcher_send_custom_event(inst->view_dispatcher, ToAppEventTypeTagWrite);
        result = true;
    }

    return result;
}

static void vb_migrate_scene_to_app_set_nfc_state(VbMigrate* inst, ToAppState state) {
    BantBlock* bant = vb_tag_get_bant_block(&inst->nfc_dev->dev_data);
    if(state == ToAppStateEmulateReady) {
        vb_tag_set_status(bant, VbTagStatusReady);
        vb_tag_set_operation(bant, VbTagOperationReady);
    }
}

static bool vb_migrate_scene_to_app_is_state_changed(VbMigrate* inst, ToAppState state) {
    BantBlock* bant = vb_tag_get_bant_block(&inst->nfc_dev->dev_data);
    VbTagOperation operation = vb_tag_get_operation(bant);

    if(state == ToAppStateEmulateReady) {
        return operation == VbTagOperationTransferToApp;
    }

    return false;
}

static void vb_migrate_scene_to_app_set_state(VbMigrate* inst, ToAppState state) {
    uint32_t curr_state = scene_manager_get_scene_state(inst->scene_manager, VbMigrateSceneToApp);
    if(state != curr_state) {
        if(state == ToAppStateInstructions) {
            widget_reset(inst->widget);
            widget_add_string_multiline_element(
                inst->widget,
                0,
                0,
                AlignLeft,
                AlignTop,
                FontSecondary,
                "Transfer to VB Lab, save in storage, and repeat.");
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeLeft,
                "Cancel",
                vb_migrate_scene_to_app_widget_callback,
                inst);
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeRight,
                "Next",
                vb_migrate_scene_to_app_widget_callback,
                inst);

            view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);
        } else if(state == ToAppStateEmulateReady) {
            view_dispatcher_send_custom_event(inst->view_dispatcher, ToAppEventTypeEmulateStart);
        } else if(state == ToAppStateLoadError) {
            FuriString* temp_str =
                furi_string_alloc_printf("Could not load capture %03d", inst->next_id);
            widget_reset(inst->widget);
            widget_add_string_multiline_element(
                inst->widget,
                0,
                0,
                AlignLeft,
                AlignTop,
                FontPrimary,
                furi_string_get_cstr(temp_str));
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeLeft,
                "Cancel",
                vb_migrate_scene_to_app_widget_callback,
                inst);

            view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);
            furi_string_free(temp_str);
            notification_message(inst->notifications, &sequence_error);
            notification_message(inst->notifications, &sequence_set_red_255);
        } else if(state == ToAppStateComplete) {
            widget_reset(inst->widget);
            widget_add_string_multiline_element(
                inst->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, "Transfers complete.");
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeRight,
                "Done",
                vb_migrate_scene_to_app_widget_callback,
                inst);

            view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);
        } else {
            furi_crash("Unknown new state in vb_migrate_scene_to_app_set_state");
        }

        scene_manager_set_scene_state(inst->scene_manager, VbMigrateSceneToApp, state);
    }
}

static void vb_migrate_scene_to_app_load_capture(VbMigrate* inst, bool go_next) {
    if(go_next) {
        nfc_worker_stop(inst->worker);
        vb_migrate_blink_stop(inst);
        ++inst->next_id;
        ++inst->num_sent;
    }

    if(inst->num_sent == inst->num_captured) {
        vb_migrate_scene_to_app_set_state(inst, ToAppStateComplete);
    } else {
        uint32_t state = scene_manager_get_scene_state(inst->scene_manager, VbMigrateSceneToApp);
        inst->next_id = vb_migrate_get_next_id(inst, inst->text_store, inst->next_id, true);
        FuriString* temp_str =
            furi_string_alloc_printf(VB_MIGRATE_CAPTURE_FORMAT, inst->next_id, NFC_APP_EXTENSION);

        vb_migrate_show_loading_popup(inst, true);
        if(vb_migrate_load_nfc(inst, inst->text_store, furi_string_get_cstr(temp_str))) {
            furi_string_printf(
                temp_str,
                "Ready, waiting for transfer\nProgress: %d/%d",
                inst->num_sent + 1,
                inst->num_captured);
            widget_reset(inst->widget);
            widget_add_string_multiline_element(
                inst->widget,
                0,
                0,
                AlignLeft,
                AlignTop,
                FontPrimary,
                furi_string_get_cstr(temp_str));
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeLeft,
                "Cancel",
                vb_migrate_scene_to_app_widget_callback,
                inst);
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeRight,
                "Skip",
                vb_migrate_scene_to_app_widget_callback,
                inst);

            view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);

            vb_migrate_scene_to_app_set_nfc_state(inst, state);
            nfc_worker_start(
                inst->worker,
                NfcWorkerStateMfUltralightEmulate,
                &inst->nfc_dev->dev_data,
                vb_migrate_scene_to_app_worker_callback,
                inst);
            vb_migrate_blink_emulate(inst);
        } else {
            view_dispatcher_send_custom_event(
                inst->view_dispatcher, ToAppEventTypeCaptureLoadError);
        }
        vb_migrate_show_loading_popup(inst, false);
        furi_string_free(temp_str);
    }
}

void vb_migrate_scene_to_app_on_enter(void* context) {
    VbMigrate* inst = context;

    // Perform your setup here
    inst->next_id = 0;
    inst->num_sent = 0;
    scene_manager_set_scene_state(inst->scene_manager, VbMigrateSceneToApp, ToAppStateInitial);
    vb_migrate_scene_to_app_set_state(inst, ToAppStateInstructions);
}

bool vb_migrate_scene_to_app_on_event(void* context, SceneManagerEvent event) {
    VbMigrate* inst = context;
    uint32_t state = scene_manager_get_scene_state(inst->scene_manager, VbMigrateSceneToApp);
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == ToAppEventTypeWidgetLeft) {
            consumed = scene_manager_previous_scene(inst->scene_manager);
        } else if(event.event == ToAppEventTypeWidgetRight) {
            if(state == ToAppStateInstructions) {
                vb_migrate_scene_to_app_set_state(inst, ToAppStateEmulateReady);
                consumed = true;
            } else if(state == ToAppStateEmulateReady) {
                vb_migrate_scene_to_app_load_capture(inst, true);
                consumed = true;
            } else if(state == ToAppStateComplete) {
                consumed = scene_manager_previous_scene(inst->scene_manager);
            }
        } else if(event.event == ToAppEventTypeEmulateStart) {
            vb_migrate_scene_to_app_load_capture(inst, false);
        } else if(event.event == ToAppEventTypeTagWrite) {
            if(vb_migrate_scene_to_app_is_state_changed(inst, state)) {
                if(state == ToAppStateEmulateReady) {
                    notification_message(inst->notifications, &sequence_success);
                    vb_migrate_scene_to_app_load_capture(inst, true);
                    consumed = true;
                }
            }
        } else if(event.event == ToAppEventTypeCaptureLoadError) {
            vb_migrate_scene_to_app_set_state(inst, ToAppStateLoadError);
            consumed = true;
        } else {
            furi_crash("Unknown event in vb_migrate_scene_to_app_on_event");
        }
    }
    return consumed;
}

void vb_migrate_scene_to_app_on_exit(void* context) {
    VbMigrate* inst = context;

    // Perform your cleanup here
    widget_reset(inst->widget);
    nfc_worker_stop(inst->worker);
    vb_migrate_blink_stop(inst);
    notification_message_block(inst->notifications, &sequence_reset_red);
}
