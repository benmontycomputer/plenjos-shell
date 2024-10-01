#pragma once

#include <stdio.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "clock.h"

#include "audio-button.h"
#include "network-button.h"
#include "bluetooth-button.h"
#include "power-button.h"

#include "media-control.h"

typedef struct PanelTrayWindow {
    GtkCenterBox *tray_box;

    GtkBox *tray_end_box;

    Clock *clock;

    GtkButton *control_center_button;

    GtkWindow *gtk_window;
} PanelTrayWindow;

typedef struct PanelTray {
    /* GtkCenterBox *tray_box;

    GtkBox *tray_end_box; */

    PanelTrayWindow **windows;

    AudioButton *audio_button;
    NetworkButton *network_button;
    BluetoothButton *bluetooth_button;
    PowerButton *power_button;

    // GtkButton *control_center_button;

    GtkStack *stack;

    // PanelTrayMenu *menu;
    GtkPopover *control_center_popover;
    GtkBox *control_center_popover_box;

    GtkGrid *control_center_grid;

    // Clock *clock;

    MediaControl *media_control;

    GtkBox *back_box;
    GtkLabel *back_label;
    GtkButton *back_button;

    gpointer *panel;
} PanelTray;

PanelTray *panel_tray_new (gpointer *panel);
void panel_tray_update_monitors (PanelTray *self);