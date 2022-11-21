#include "../vb_migrate_i.h"

void vb_migrate_scene_about_on_enter(void* context) {
    VbMigrate* inst = context;

    // Perform your setup here
    FuriString* temp_str = furi_string_alloc_printf(
        "\e#Information\n"
        "Version: %s\n"
        "Developed by: cyanic\n"
        "GitHub: ...\n"
        "\n"
        "\e#Description\n"
        "Makes transferring\n"
        "characters with VB Lab less\n"
        "cumbersome",
        VB_MIGRATE_VERSION);

    Widget* widget = inst->widget;
    widget_add_text_box_element(
        widget,
        0,
        0,
        128,
        14,
        AlignCenter,
        AlignBottom,
        "\e#\e!                                                      \e!\n",
        false);
    widget_add_text_box_element(
        widget,
        0,
        2,
        128,
        14,
        AlignCenter,
        AlignBottom,
        "\e#\e!  VB Migration Assistant \e!\n",
        false);
    widget_add_text_scroll_element(widget, 0, 16, 128, 50, furi_string_get_cstr(temp_str));
    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(inst->view_dispatcher, VbMigrateViewWidget);
}

bool vb_migrate_scene_about_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void vb_migrate_scene_about_on_exit(void* context) {
    VbMigrate* inst = context;

    // Perform your cleanup here
    widget_reset(inst->widget);
}
