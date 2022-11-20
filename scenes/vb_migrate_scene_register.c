#include <notification/notification_messages.h>

#include "../vb_tag.h"
#include "../vb_migrate_i.h"

typedef enum {
    RegisterStateInitial,
    RegisterStateInstructionInitial,
    RegisterStateInstructionConnect,
    RegisterStateCaptureInitial,
    RegisterStateCaptureInvalidTag,
    RegisterStateCapturePwd,
    RegisterStateCaptureFull,
    RegisterStateCaptureFailed,
    RegisterStateCaptureIncorrectTag,
} RegisterState;

typedef enum {
    RegisterEventTypeNextButton,
    RegisterEventTypePrevButton,
    RegisterEventTypeVbReadInitial,
    RegisterEventTypeVbPwdAuth,
    RegisterEventTypeVbReadFullSuccess,
    RegisterEventTypeVbReadFullFail,
} RegisterEventType;

static void
    vb_migrate_scene_register_widget_callback(GuiButtonType result, InputType type, void* context) {
    VbMigrate* inst = context;
    UNUSED(result);

    if(type == InputTypeShort) {
        if(result == GuiButtonTypeRight)
            view_dispatcher_send_custom_event(inst->view_dispatcher, RegisterEventTypeNextButton);
        else if(result == GuiButtonTypeLeft)
            view_dispatcher_send_custom_event(inst->view_dispatcher, RegisterEventTypePrevButton);
    }
}

static bool
    vb_migrate_scene_register_worker_read_initial_callback(NfcWorkerEvent event, void* context) {
    VbMigrate* inst = context;
    bool consumed = false;

    if(event == NfcWorkerEventReadMfUltralight) {
        view_dispatcher_send_custom_event(inst->view_dispatcher, RegisterEventTypeVbReadInitial);
        consumed = true;
    }

    return consumed;
}

static bool vb_migrate_scene_register_worker_auth_callback(NfcWorkerEvent event, void* context) {
    VbMigrate* inst = context;
    bool consumed = false;

    if(event == NfcWorkerEventMfUltralightPwdAuth) {
        view_dispatcher_send_custom_event(inst->view_dispatcher, RegisterEventTypeVbPwdAuth);
        consumed = true;
    }

    return consumed;
}

static bool
    vb_migrate_scene_register_worker_full_capture_callback(NfcWorkerEvent event, void* context) {
    VbMigrate* inst = context;
    bool consumed = false;

    if(event == NfcWorkerEventMfUltralightPassKey) {
        memcpy(
            inst->nfc_dev->dev_data.mf_ul_data.auth_key,
            inst->captured_pwd,
            sizeof(inst->captured_pwd));
        consumed = true;
    } else if(event == NfcWorkerEventSuccess) {
        view_dispatcher_send_custom_event(
            inst->view_dispatcher, RegisterEventTypeVbReadFullSuccess);
        consumed = true;
    } else if(event == NfcWorkerEventFail) {
        view_dispatcher_send_custom_event(inst->view_dispatcher, RegisterEventTypeVbReadFullFail);
        consumed = true;
    }

    return consumed;
}

static void vb_migrate_scene_register_cleanup_state(VbMigrate* inst, RegisterState state) {
    if(state == RegisterStateCaptureInvalidTag || state == RegisterStateCaptureFailed ||
       state == RegisterStateCaptureIncorrectTag) {
        notification_message(inst->notifications, &sequence_reset_red);
    } else if(
        state == RegisterStateCaptureInitial || state == RegisterStateCapturePwd ||
        state == RegisterStateCaptureFull) {
        vb_migrate_blink_stop(inst);
        nfc_worker_stop(inst->worker);
    }
}

static void vb_migrate_scene_register_set_state(VbMigrate* inst, RegisterState state) {
    uint32_t curr_state =
        scene_manager_get_scene_state(inst->scene_manager, VbMigrateSceneRegister);
    if(state != curr_state) {
        vb_migrate_scene_register_cleanup_state(inst, curr_state);

        if(state == RegisterStateInstructionInitial) {
            widget_reset(inst->widget);
            widget_add_string_multiline_element(
                inst->widget,
                0,
                0,
                AlignLeft,
                AlignTop,
                FontSecondary,
                "Please make sure your current character has been sent to the app before continuing.");
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeLeft,
                "Cancel",
                vb_migrate_scene_register_widget_callback,
                inst);
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeRight,
                "Next",
                vb_migrate_scene_register_widget_callback,
                inst);

            view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);
        } else if(state == RegisterStateInstructionConnect) {
            widget_reset(inst->widget);
            widget_add_string_multiline_element(
                inst->widget,
                0,
                0,
                AlignLeft,
                AlignTop,
                FontSecondary,
                "Prepare VB Lab:\n1. Open the \"Scan\" screen\n2. Tap \"Vital Bracelet -> App\"");
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeLeft,
                "Cancel",
                vb_migrate_scene_register_widget_callback,
                inst);
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeRight,
                "Next",
                vb_migrate_scene_register_widget_callback,
                inst);

            view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);
        } else if(state == RegisterStateCaptureInitial) {
            widget_reset(inst->widget);
            widget_add_string_multiline_element(
                inst->widget,
                0,
                0,
                AlignLeft,
                AlignTop,
                FontPrimary,
                "Tap Flipper to your Vital Bracelet.");
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeLeft,
                "Previous",
                vb_migrate_scene_register_widget_callback,
                inst);

            view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);
            nfc_device_clear(inst->nfc_dev);
            inst->nfc_dev->dev_data.read_mode = NfcReadModeMfUltralight;
            nfc_worker_start(
                inst->worker,
                NfcWorkerStateRead,
                &inst->nfc_dev->dev_data,
                vb_migrate_scene_register_worker_read_initial_callback,
                inst);
            vb_migrate_blink_read(inst);
        } else if(state == RegisterStateCaptureInvalidTag) {
            widget_reset(inst->widget);
            widget_add_string_multiline_element(
                inst->widget,
                0,
                0,
                AlignLeft,
                AlignTop,
                FontSecondary,
                "Tag is not a valid Vital Bracelet, please try again.");
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeLeft,
                "Retry",
                vb_migrate_scene_register_widget_callback,
                inst);

            view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);
            notification_message(inst->notifications, &sequence_set_red_255);
        } else if(state == RegisterStateCapturePwd) {
            widget_reset(inst->widget);
            widget_add_string_multiline_element(
                inst->widget,
                0,
                0,
                AlignLeft,
                AlignTop,
                FontPrimary,
                "Tap Send on VB Lab, and tap Flipper to phone.");
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeLeft,
                "Previous",
                vb_migrate_scene_register_widget_callback,
                inst);

            view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);

            BantBlock* bant = vb_tag_get_bant_block(&inst->nfc_dev->dev_data);
            vb_tag_set_operation(bant, VbTagOperationReady);
            vb_tag_set_status(bant, VbTagStatusReady);
            nfc_worker_start(
                inst->worker,
                NfcWorkerStateMfUltralightEmulate,
                &inst->nfc_dev->dev_data,
                vb_migrate_scene_register_worker_auth_callback,
                inst);
            vb_migrate_blink_emulate(inst);
        } else if(state == RegisterStateCaptureFull) {
            widget_reset(inst->widget);
            widget_add_string_multiline_element(
                inst->widget,
                0,
                0,
                AlignLeft,
                AlignTop,
                FontPrimary,
                "Tap Flipper to your Vital Bracelet again.");
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeLeft,
                "Previous",
                vb_migrate_scene_register_widget_callback,
                inst);

            view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);

            inst->nfc_dev->dev_data.mf_ul_data.auth_method = MfUltralightAuthMethodAuto;
            nfc_worker_start(
                inst->worker,
                NfcWorkerStateReadMfUltralightReadAuth,
                &inst->nfc_dev->dev_data,
                vb_migrate_scene_register_worker_full_capture_callback,
                inst);
            vb_migrate_blink_read(inst);
        } else if(state == RegisterStateCaptureFailed) {
            widget_reset(inst->widget);
            widget_add_string_multiline_element(
                inst->widget,
                0,
                0,
                AlignLeft,
                AlignTop,
                FontSecondary,
                "Failed to read VB, please try again.");
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeLeft,
                "Retry",
                vb_migrate_scene_register_widget_callback,
                inst);

            view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);
            notification_message(inst->notifications, &sequence_set_red_255);
        } else if(state == RegisterStateCaptureIncorrectTag) {
            widget_reset(inst->widget);
            widget_add_string_multiline_element(
                inst->widget,
                0,
                0,
                AlignLeft,
                AlignTop,
                FontSecondary,
                "Different tag read, please try again.");
            widget_add_button_element(
                inst->widget,
                GuiButtonTypeLeft,
                "Retry",
                vb_migrate_scene_register_widget_callback,
                inst);

            view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);
            notification_message(inst->notifications, &sequence_set_red_255);
        } else {
            furi_crash("Unknown new state in vb_migrate_scene_register_set_state");
        }

        scene_manager_set_scene_state(inst->scene_manager, VbMigrateSceneRegister, state);
    }
}

static bool vb_migrate_scene_register_next_state(VbMigrate* inst, RegisterState state) {
    if(state == RegisterStateInstructionInitial) {
        vb_migrate_scene_register_set_state(inst, RegisterStateInstructionConnect);
        return true;
    } else if(state == RegisterStateInstructionConnect) {
        vb_migrate_scene_register_set_state(inst, RegisterStateCaptureInitial);
        return true;
    }

    return false;
}

static bool
    vb_migrate_scene_register_prev_state(VbMigrate* inst, RegisterState state, bool is_back) {
    UNUSED(is_back);

    if(state == RegisterStateInstructionInitial || state == RegisterStateInstructionConnect) {
        return scene_manager_previous_scene(inst->scene_manager);
    } else if(state == RegisterStateCaptureInitial) {
        vb_migrate_scene_register_set_state(inst, RegisterStateInstructionConnect);
        return true;
    } else if(
        state == RegisterStateCaptureInvalidTag || state == RegisterStateCapturePwd ||
        state == RegisterStateCaptureFull) {
        vb_migrate_scene_register_set_state(inst, RegisterStateCaptureInitial);
        return true;
    } else if(state == RegisterStateCaptureFailed || state == RegisterStateCaptureIncorrectTag) {
        vb_migrate_scene_register_set_state(inst, RegisterStateCaptureFull);
        return true;
    }

    return is_back;
}

void vb_migrate_scene_register_on_enter(void* context) {
    VbMigrate* inst = context;

    scene_manager_set_scene_state(
        inst->scene_manager, VbMigrateSceneRegister, RegisterStateInitial);
    vb_migrate_scene_register_set_state(inst, RegisterStateInstructionInitial);
}

bool vb_migrate_scene_register_on_event(void* context, SceneManagerEvent event) {
    VbMigrate* inst = context;
    bool consumed = false;
    RegisterState state =
        scene_manager_get_scene_state(inst->scene_manager, VbMigrateSceneRegister);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == RegisterEventTypeNextButton) {
            consumed = vb_migrate_scene_register_next_state(inst, state);
        } else if(event.event == RegisterEventTypePrevButton) {
            consumed = vb_migrate_scene_register_prev_state(inst, state, false);
        } else if(event.event == RegisterEventTypeVbReadInitial) {
            if(vb_tag_validate_product(&inst->nfc_dev->dev_data)) {
                memcpy(
                    inst->captured_uid,
                    inst->nfc_dev->dev_data.nfc_data.uid,
                    sizeof(inst->captured_uid));
                notification_message(inst->notifications, &sequence_success);
                vb_migrate_scene_register_set_state(inst, RegisterStateCapturePwd);
            } else {
                notification_message(inst->notifications, &sequence_error);
                vb_migrate_scene_register_set_state(inst, RegisterStateCaptureInvalidTag);
            }
            consumed = true;
        } else if(event.event == RegisterEventTypeVbPwdAuth) {
            // Set up for auth
            memcpy(
                inst->captured_pwd,
                inst->nfc_dev->dev_data.mf_ul_auth.pwd.raw,
                sizeof(inst->captured_pwd));

            notification_message(inst->notifications, &sequence_success);
            vb_migrate_scene_register_set_state(inst, RegisterStateCaptureFull);
            consumed = true;
        } else if(event.event == RegisterEventTypeVbReadFullSuccess) {
            NfcDeviceData* dev_data = &inst->nfc_dev->dev_data;
            if(memcmp(dev_data->nfc_data.uid, inst->captured_uid, sizeof(inst->captured_uid)) ||
               dev_data->mf_ul_data.data_read != dev_data->mf_ul_data.data_size) {
                notification_message(inst->notifications, &sequence_error);
                vb_migrate_scene_register_set_state(inst, RegisterStateCaptureIncorrectTag);
            } else {
                notification_message(inst->notifications, &sequence_success);
                scene_manager_next_scene(inst->scene_manager, VbMigrateSceneRegisterSave);
            }
            consumed = true;
        } else if(event.event == RegisterEventTypeVbReadFullFail) {
            notification_message(inst->notifications, &sequence_error);
            vb_migrate_scene_register_set_state(inst, RegisterStateCaptureFailed);
            consumed = true;
        } else {
            furi_crash("Unknown event in vb_migrate_scene_register_on_event");
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = vb_migrate_scene_register_prev_state(inst, state, true);
    }
    return consumed;
}

void vb_migrate_scene_register_on_exit(void* context) {
    VbMigrate* inst = context;
    RegisterState state =
        scene_manager_get_scene_state(inst->scene_manager, VbMigrateSceneRegister);

    vb_migrate_scene_register_cleanup_state(inst, state);
    widget_reset(inst->widget);
}
