#include <notification/notification_messages.h>

#include "../vb_migrate_i.h"
#include "../vb_tag.h"

#define TAG "vb_migrate_scene_from_app"

typedef enum {
    FromAppStateInitial,
    FromAppStateInstructions,
    FromAppStateEmulateReady,
    FromAppStateEmulateCheckDim,
    FromAppStateEmulateTransferFromApp,
    FromAppStateTemplateError,
    FromAppStateSaveError,
} FromAppState;

typedef enum {
    FromAppEventTypeWidgetLeft,
    FromAppEventTypeWidgetRight,
    FromAppEventTypeTemplateLoadError,
    FromAppEventTypeTagWrite,
    FromAppEventTypeCaptureSaveError,
    FromAppEventTypeCaptureSaveSuccess,
} FromAppEventType;

static void
    vb_migrate_scene_from_app_widget_callback(GuiButtonType result, InputType type, void* context) {
    VbMigrate* inst = context;

    if(type == InputTypeShort) {
        if(result == GuiButtonTypeLeft)
            view_dispatcher_send_custom_event(inst->view_dispatcher, FromAppEventTypeWidgetLeft);
        else if(result == GuiButtonTypeRight)
            view_dispatcher_send_custom_event(inst->view_dispatcher, FromAppEventTypeWidgetRight);
    }
}

static bool vb_migrate_scene_from_app_worker_callback(NfcWorkerEvent event, void* context) {
    VbMigrate* inst = context;
    bool result = false;

    if(event == NfcWorkerEventSuccess) {
        view_dispatcher_send_custom_event(inst->view_dispatcher, FromAppEventTypeTagWrite);
        result = true;
    }

    return result;
}

static void vb_migrate_scene_from_app_set_nfc_state(VbMigrate* inst, FromAppState state) {
    BantBlock* bant = vb_tag_get_bant_block(&inst->nfc_dev->dev_data);
    if(state == FromAppStateEmulateReady) {
        vb_tag_set_status(bant, VbTagStatusReady);
        vb_tag_set_operation(bant, VbTagOperationReady);
    } else if(state == FromAppStateEmulateCheckDim) {
        vb_tag_set_status(bant, VbTagStatusReady | VbTagStatusDimReady);
        vb_tag_set_operation(bant, VbTagOperationReady);
    } else if(state == FromAppStateEmulateTransferFromApp) {
        vb_tag_set_status(bant, 0);
        vb_tag_set_operation(bant, VbTagOperationIdle);
    }
}

static bool vb_migrate_scene_from_app_is_state_changed(VbMigrate* inst, FromAppState state) {
    BantBlock* bant = vb_tag_get_bant_block(&inst->nfc_dev->dev_data);
    VbTagOperation operation = vb_tag_get_operation(bant);

    if(state == FromAppStateEmulateReady) {
        return operation == VbTagOperationCheckDim;
    } else if(state == FromAppStateEmulateCheckDim) {
        return operation == VbTagOperationReturnFromApp;
    }

    return false;
}

static void vb_migrate_scene_from_app_set_state(VbMigrate* inst, FromAppState state) {
    uint32_t curr_state =
        scene_manager_get_scene_state(inst->scene_manager, VbMigrateSceneFromApp);
    if(state != curr_state) {
        if(state == FromAppStateInstructions) {
            widget_reset(inst->widget);
            widget_add_string_multiline_element(
                inst->widget,
                0,
                0,
                AlignLeft,
                AlignTop,
                FontSecondary,
                "Wake up and send your character on the VB Lab app, using Flipper as if it is a "
                "Vital Bracelet. Just tap phone to Flipper directly when needed.");
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeLeft,
                "Cancel",
                vb_migrate_scene_from_app_widget_callback,
                inst);
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeRight,
                "Next",
                vb_migrate_scene_from_app_widget_callback,
                inst);

            view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);
        } else if(state == FromAppStateEmulateReady) {
            vb_migrate_show_loading_popup(inst, true);
            if(vb_migrate_load_nfc(inst, inst->text_store, VB_MIGRATE_TEMPLATE_NAME)) {
                FuriString* temp_str = furi_string_alloc_printf(
                    "Ready, waiting for Dim check\nNum. captured: %d", inst->num_captured);
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
                    vb_migrate_scene_from_app_widget_callback,
                    inst);

                view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);
                furi_string_free(temp_str);

                vb_migrate_scene_from_app_set_nfc_state(inst, state);
                nfc_worker_start(
                    inst->worker,
                    NfcWorkerStateMfUltralightEmulate,
                    &inst->nfc_dev->dev_data,
                    vb_migrate_scene_from_app_worker_callback,
                    inst);
                vb_migrate_blink_emulate(inst);
            } else {
                view_dispatcher_send_custom_event(
                    inst->view_dispatcher, FromAppEventTypeTemplateLoadError);
            }
            vb_migrate_show_loading_popup(inst, false);
        } else if(state == FromAppStateEmulateCheckDim) {
            widget_reset(inst->widget);
            widget_add_string_multiline_element(
                inst->widget,
                0,
                0,
                AlignLeft,
                AlignTop,
                FontPrimary,
                "Waiting for second Dim check/transfer");
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeLeft,
                "Cancel",
                vb_migrate_scene_from_app_widget_callback,
                inst);

            view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);
            notification_message(inst->notifications, &sequence_success);
            vb_migrate_scene_from_app_set_nfc_state(inst, state);
            nfc_worker_start(
                inst->worker,
                NfcWorkerStateMfUltralightEmulate,
                &inst->nfc_dev->dev_data,
                vb_migrate_scene_from_app_worker_callback,
                inst);
            vb_migrate_blink_emulate(inst);
        } else if(state == FromAppStateEmulateTransferFromApp) {
            widget_reset(inst->widget);
            widget_add_string_multiline_element(
                inst->widget,
                0,
                0,
                AlignLeft,
                AlignTop,
                FontPrimary,
                "Transfer captured, saving...");
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeLeft,
                "Cancel",
                vb_migrate_scene_from_app_widget_callback,
                inst);

            view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);
            nfc_worker_stop(inst->worker);
            vb_migrate_blink_stop(inst);
            vb_migrate_scene_from_app_set_nfc_state(inst, state);
            notification_message(inst->notifications, &sequence_success);

            // Save the tag
            inst->next_id = vb_migrate_get_next_id(inst, inst->text_store, inst->next_id, false);
            FuriString* save_path =
                furi_string_alloc_printf("%03d%s", inst->next_id, NFC_APP_EXTENSION);
            if(vb_migrate_save_nfc(inst, inst->text_store, furi_string_get_cstr(save_path))) {
                view_dispatcher_send_custom_event(
                    inst->view_dispatcher, FromAppEventTypeCaptureSaveSuccess);
            } else {
                view_dispatcher_send_custom_event(
                    inst->view_dispatcher, FromAppEventTypeCaptureSaveError);
            }
            furi_string_free(save_path);
        } else if(state == FromAppStateTemplateError) {
            widget_reset(inst->widget);
            widget_add_string_multiline_element(
                inst->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, "Could not load template");
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeLeft,
                "Cancel",
                vb_migrate_scene_from_app_widget_callback,
                inst);

            view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);
            notification_message(inst->notifications, &sequence_error);
            notification_message(inst->notifications, &sequence_set_red_255);
        } else if(state == FromAppStateSaveError) {
            widget_reset(inst->widget);
            widget_add_string_multiline_element(
                inst->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, "Could not save capture");
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeLeft,
                "Cancel",
                vb_migrate_scene_from_app_widget_callback,
                inst);

            view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);
            notification_message(inst->notifications, &sequence_error);
            notification_message(inst->notifications, &sequence_set_red_255);
        } else {
            furi_crash("Unknown new state in vb_migrate_scene_from_app_set_state");
        }

        scene_manager_set_scene_state(inst->scene_manager, VbMigrateSceneFromApp, state);
    }
}

void vb_migrate_scene_from_app_on_enter(void* context) {
    VbMigrate* inst = context;

    // Perform your setup here
    inst->next_id = 0;
    scene_manager_set_scene_state(inst->scene_manager, VbMigrateSceneFromApp, FromAppStateInitial);
    vb_migrate_scene_from_app_set_state(inst, FromAppStateInstructions);
}

bool vb_migrate_scene_from_app_on_event(void* context, SceneManagerEvent event) {
    VbMigrate* inst = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == FromAppEventTypeWidgetLeft) {
            consumed = scene_manager_previous_scene(inst->scene_manager);
        } else if(event.event == FromAppEventTypeWidgetRight) {
            vb_migrate_scene_from_app_set_state(inst, FromAppStateEmulateReady);
            consumed = true;
        } else if(event.event == FromAppEventTypeTagWrite) {
            uint32_t state =
                scene_manager_get_scene_state(inst->scene_manager, VbMigrateSceneFromApp);
            if(vb_migrate_scene_from_app_is_state_changed(inst, state)) {
                if(state == FromAppStateEmulateReady) {
                    nfc_worker_stop(inst->worker);
                    vb_migrate_blink_stop(inst);
                    vb_migrate_scene_from_app_set_state(inst, FromAppStateEmulateCheckDim);
                    consumed = true;
                } else if(state == FromAppStateEmulateCheckDim) {
                    nfc_worker_stop(inst->worker);
                    vb_migrate_blink_stop(inst);
                    vb_migrate_scene_from_app_set_state(inst, FromAppStateEmulateTransferFromApp);
                    consumed = true;
                }
            }
        } else if(event.event == FromAppEventTypeTemplateLoadError) {
            vb_migrate_scene_from_app_set_state(inst, FromAppStateTemplateError);
            consumed = true;
        } else if(event.event == FromAppEventTypeCaptureSaveError) {
            vb_migrate_scene_from_app_set_state(inst, FromAppStateSaveError);
            consumed = true;
        } else if(event.event == FromAppEventTypeCaptureSaveSuccess) {
            ++inst->num_captured;
            ++inst->next_id;
            vb_migrate_scene_from_app_set_state(inst, FromAppStateEmulateReady);
            consumed = true;
        } else {
            furi_crash("Unknown event in vb_migrate_scene_from_app_on_event");
        }
    }
    return consumed;
}

void vb_migrate_scene_from_app_on_exit(void* context) {
    VbMigrate* inst = context;

    // Perform your cleanup here
    widget_reset(inst->widget);
    nfc_worker_stop(inst->worker);
    vb_migrate_blink_stop(inst);
    notification_message_block(inst->notifications, &sequence_reset_red);
}
