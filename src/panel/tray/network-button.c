#include "network-button.h"

NetworkButton *network_button_new () {
    NetworkButton *self = malloc (sizeof (NetworkButton));

    self->button = gtk_button_new_from_icon_name ("network-wired-symbolic", GTK_ICON_SIZE_DND);

    gtk_widget_set_name (self->button, "panel_tray_menu_button");

    return self;
}