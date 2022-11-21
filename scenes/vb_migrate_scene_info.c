#include "../vb_migrate_i.h"
#include "../vb_tag.h"

void vb_migrate_scene_info_button_callback(GuiButtonType result, InputType type, void* context) {
    VbMigrate* inst = context;

    if(type == InputTypeShort) {
        if(result == GuiButtonTypeRight) {
            view_dispatcher_send_custom_event(inst->view_dispatcher, result);
        }
    }
}

void vb_migrate_scene_info_on_enter(void* context) {
    VbMigrate* inst = context;
    FuriString* temp_str = furi_string_alloc();

    // Build info scroll
    // Name
    furi_string_cat_printf(temp_str, "\ec%s\n", inst->text_store);

    // Type
    BantBlock* bant = vb_tag_get_bant_block(&inst->nfc_dev->dev_data);
    const VbTagProduct* product = vb_tag_find_product(bant);
    if(product == NULL)
        furi_string_cat(temp_str, "Unknown product\n");
    else
        furi_string_cat_printf(temp_str, "\e#%s\n", product->name);

    // Number of mons loaded
    int count = vb_migrate_count_captured_mons(inst, inst->text_store);
    furi_string_cat_printf(temp_str, "Num. captured: %d", count);

    widget_add_text_scroll_element(inst->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));
    furi_string_free(temp_str);

    widget_add_button_element(
        inst->widget, GuiButtonTypeRight, "More", vb_migrate_scene_info_button_callback, inst);
    view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);
}

bool vb_migrate_scene_info_on_event(void* context, SceneManagerEvent event) {
    VbMigrate* inst = context;
    bool consumed = false;
    UNUSED(inst);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(inst->scene_manager, VbMigrateSceneDevMenu);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        uint32_t back_scenes[] = {VbMigrateSceneSelect, VbMigrateSceneMainMenu};
        consumed = scene_manager_search_and_switch_to_previous_scene_one_of(
            inst->scene_manager, back_scenes, COUNT_OF(back_scenes));
    }
    return consumed;
}

void vb_migrate_scene_info_on_exit(void* context) {
    VbMigrate* inst = context;

    // Perform your cleanup here
    widget_reset(inst->widget);
}