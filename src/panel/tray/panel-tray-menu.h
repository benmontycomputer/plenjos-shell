#pragma once

#include <stdbool.h>
#include <stdio.h>

#include <gtk/gtk.h>

#include <gtk4-layer-shell.h>

#include "../panel-common.h"

typedef struct PanelTrayMenu {
    GtkBox *box;

    GtkWindow *window;

    bool visible;

    Panel *panel;
} PanelTrayMenu;

PanelTrayMenu *panel_tray_menu_new (Panel *panel);

void panel_tray_menu_toggle_show (PanelTrayMenu *self);