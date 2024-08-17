#include "audio-button.h"

AudioButton *audio_button_new () {
    AudioButton *self = malloc (sizeof (AudioButton));

    self->button = gtk_button_new_from_icon_name ("audio-volume-medium", GTK_ICON_SIZE_DND);

    gtk_widget_set_name (self->button, "panel_button");

    return self;
}