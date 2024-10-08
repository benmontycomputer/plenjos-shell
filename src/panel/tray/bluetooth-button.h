#pragma once

#include <gtk/gtk.h>
#include <bluetooth-settings-widget.h>

#include "../panel.h"

typedef struct BluetoothButton {
    GtkStack *stack;

    GtkBox *box;

    GtkButton *button;

    BluetoothSettingsWidget *settings;
} BluetoothButton;

BluetoothButton *bluetooth_button_new (GtkStack *stack);