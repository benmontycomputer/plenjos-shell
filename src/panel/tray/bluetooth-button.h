#pragma once

#include <gtk/gtk.h>

#include "../panel.h"

typedef struct BluetoothButton {
    GtkStack *stack;

    GtkBox *box;

    GtkButton *button;
} BluetoothButton;

BluetoothButton *bluetooth_button_new (GtkStack *stack);