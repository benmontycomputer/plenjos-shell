#include "network-button.h"

NetworkButton *network_button_new () {
    NetworkButton *self = malloc (sizeof (NetworkButton));

    self->button = gtk_button_new_from_icon_name ("network-wired", GTK_ICON_SIZE_DND);

    gtk_widget_set_name (self->button, "panel_button");

    return self;
}