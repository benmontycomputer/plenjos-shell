#pragma once

#include <gtk/gtk.h>
#include <glib.h>

#include "../panel.h"

typedef struct PowerButton {
    GtkButton *button;

    GtkStack *stack;

    GtkBox *box;

    GtkButton *shutdown;
    GtkButton *restart;
    GtkButton *lock;
    GtkButton *logout;
} PowerButton;

PowerButton *power_button_new (GtkStack *stack);