#pragma once

#include <gtk/gtk.h>

#include <alsa/asoundlib.h>

#include "../panel.h"

typedef struct AudioButton {
    GtkStack *stack;

    GtkBox *box;

    GtkButton *button;

    GtkScale *volume;

    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    snd_mixer_elem_t *elem;
} AudioButton;

AudioButton *audio_button_new (GtkStack *stack);