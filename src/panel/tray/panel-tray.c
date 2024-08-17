#include "panel-tray.h"

void
show_control_center (GtkButton *button, PanelTray *self) {
    gtk_stack_set_visible_child_name (self->stack, "control-center");

    panel_tray_menu_toggle_show (self->menu, 0, 4);
}

PanelTray *
panel_tray_new (gpointer panel_ptr) {
    PanelTray *self = malloc (sizeof (PanelTray));

    self->stack = GTK_STACK (gtk_stack_new ());

    self->control_center_grid = GTK_GRID (gtk_grid_new ());

    gtk_widget_set_hexpand (GTK_WIDGET (self->control_center_grid), TRUE);
    gtk_widget_set_vexpand (GTK_WIDGET (self->control_center_grid), TRUE);

    gtk_grid_set_row_homogeneous (self->control_center_grid, TRUE);
    gtk_grid_set_column_homogeneous (self->control_center_grid, TRUE);

    self->control_center_button = GTK_BUTTON (
        gtk_button_new_from_icon_name ("tweaks-app", GTK_ICON_SIZE_DND));

    gtk_widget_set_name (self->control_center_button, "panel_button");

    g_signal_connect (self->control_center_button, "clicked",
                      show_control_center, self);

    gtk_stack_add_titled (self->stack, GTK_WIDGET (self->control_center_grid),
                          "control-center", "Control Center");

    self->media_control = media_control_new ();

    self->menu = panel_tray_menu_new (panel_ptr);

    gtk_box_pack_start (self->menu->box, GTK_WIDGET (self->media_control->box),
                        FALSE, FALSE, 0);
    gtk_box_pack_start (self->menu->box, GTK_WIDGET (self->stack), FALSE,
                        FALSE, 0);

    gtk_widget_show_all (GTK_WIDGET (self->menu->box));

    self->audio_button = audio_button_new (self->stack);
    self->network_button = network_button_new ();
    self->power_button = power_button_new (self->stack);

    self->box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));

    self->clock = clock_new ();

    g_timeout_add (500, (GSourceFunc)clock_update, self->clock);

    gtk_grid_attach (self->control_center_grid,
                     GTK_WIDGET (self->audio_button->button), 0, 0, 1, 1);
    gtk_grid_attach (self->control_center_grid,
                     GTK_WIDGET (self->network_button->button), 1, 0, 1, 1);
    gtk_grid_attach (self->control_center_grid,
                     GTK_WIDGET (self->power_button->button), 0, 1, 1, 1);

    gtk_box_pack_start (self->box, GTK_WIDGET (self->control_center_button),
                        FALSE, FALSE, 0);
    gtk_box_pack_start (self->box, GTK_WIDGET (self->clock->label), FALSE,
                        FALSE, 0);

    gtk_widget_show_all (GTK_WIDGET (self->control_center_grid));

    return self;
}