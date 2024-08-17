#include "power-button.h"

static void toggle_menu_wrap (GtkButton *button, PowerButton *self) {
    panel_tray_menu_toggle_show (self->menu, 0, 4);
}

GtkButton *power_button_menu_button_new (char *icon, char *label) {
    GtkBox *box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4));

    gtk_box_pack_start (box, gtk_image_new_from_icon_name (icon, GTK_ICON_SIZE_BUTTON), FALSE, FALSE, 0);
    gtk_box_pack_start (box, gtk_label_new (label), FALSE, FALSE, 0);

    gtk_widget_show_all (GTK_WIDGET (box));

    GtkButton *button = GTK_BUTTON (gtk_button_new ());

    gtk_container_add (button, GTK_WIDGET (box));

    return button;
}

PowerButton *power_button_new (gpointer panel_ptr) {
    PowerButton *self = malloc (sizeof (PowerButton));

    self->button = gtk_button_new_from_icon_name ("system-shutdown", GTK_ICON_SIZE_DND);

    gtk_widget_set_name (self->button, "panel_button");

    self->menu = panel_tray_menu_new (panel_ptr);

    g_signal_connect (self->button, "clicked", toggle_menu_wrap, self);

    self->shutdown = power_button_menu_button_new ("system-shutdown", "Shut down");
    self->restart = power_button_menu_button_new ("system-reboot", "Restart");
    self->lock = power_button_menu_button_new ("system-lock-screen", "Lock");
    self->logout = power_button_menu_button_new ("system-log-out", "Log out");

    gtk_box_pack_start (self->menu->box, GTK_WIDGET (self->shutdown), FALSE, FALSE, 0);
    gtk_box_pack_start (self->menu->box, GTK_WIDGET (self->restart), FALSE, FALSE, 0);
    gtk_box_pack_start (self->menu->box, GTK_WIDGET (self->lock), FALSE, FALSE, 0);
    gtk_box_pack_start (self->menu->box, GTK_WIDGET (self->logout), FALSE, FALSE, 0);

    gtk_widget_show_all (GTK_WIDGET (self->menu->box));

    return self;
}