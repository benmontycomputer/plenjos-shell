#include "power-button.h"

PowerButton *power_button_new () {
    PowerButton *self = malloc (sizeof (PowerButton));

    self->button = gtk_button_new_from_icon_name ("system-shutdown", GTK_ICON_SIZE_DND);

    gtk_widget_set_name (self->button, "panel_button");

    return self;
}