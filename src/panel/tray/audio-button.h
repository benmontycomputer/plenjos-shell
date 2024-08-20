#pragma once

#include <gtk/gtk.h>

#include "../panel.h"

typedef struct AudioButton {
    GtkStack *stack;

    GtkBox *box;

    GtkButton *button;

    GtkScale *volume;
} AudioButton;

AudioButton *audio_button_new (GtkStack *stack);