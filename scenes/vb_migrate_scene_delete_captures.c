#include "../vb_migrate_i.h"

static void vb_migrate_scene_delete_captures_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    VbMigrate* inst = context;

    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(inst->view_dispatcher, result);
    }
}

void vb_migrate_scene_delete_captures_on_enter(void* context) {
    VbMigrate* inst = context;

    // Perform your setup here
    widget_add_text_box_element(
        inst->widget, 0, 0, 128, 23, AlignCenter, AlignCenter, "\e#Clear captures?\e#", false);
    widget_add_button_element(
        inst->widget,
        GuiButtonTypeLeft,
        "Cancel",
        vb_migrate_scene_delete_captures_widget_callback,
        inst);
    widget_add_button_element(
        inst->widget,
        GuiButtonTypeRight,
        "Delete",
        vb_migrate_scene_delete_captures_widget_callback,
        inst);

    FuriString* temp_str =
        furi_string_alloc_printf("%s\nCharas. captured: %d", inst->text_store, inst->num_captured);
    widget_add_string_multiline_element(
        inst->widget, 64, 24, AlignCenter, AlignTop, FontSecondary, furi_string_get_cstr(temp_str));
    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);
}

bool vb_migrate_scene_delete_captures_on_event(void* context, SceneManagerEvent event) {
    VbMigrate* inst = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(inst->scene_manager);
        } else if(event.event == GuiButtonTypeRight) {
            if(vb_migrate_delete(inst, inst->text_store, false)) {
                scene_manager_next_scene(inst->scene_manager, VbMigrateSceneDeleteCapturesSuccess);
                consumed = true;
            } else {
                consumed = scene_manager_previous_scene(inst->scene_manager);
            }
            inst->num_captured = vb_migrate_count_captured_mons(inst, inst->text_store);
        }
    }
    return consumed;
}

void vb_migrate_scene_delete_captures_on_exit(void* context) {
    VbMigrate* inst = context;

    // Perform your cleanup here
    widget_reset(inst->widget);
}
