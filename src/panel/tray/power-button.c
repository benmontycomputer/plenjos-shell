#include "power-button.h"

void toggle_menu_wrap (GtkButton *button, PowerButton *self) {
    panel_tray_menu_toggle_show (self->menu, 0, 8);
}

PowerButton *power_button_new () {
    PowerButton *self = malloc (sizeof (PowerButton));

    self->button = gtk_button_new_from_icon_name ("system-shutdown", GTK_ICON_SIZE_DND);

    gtk_widget_set_name (self->button, "panel_button");

    self->menu = panel_tray_menu_new ();

    g_signal_connect (self->button, "clicked", toggle_menu_wrap, self);

    return self;
}