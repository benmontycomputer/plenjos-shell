#pragma once

#include <stdbool.h>
#include <stdio.h>

#include <gtk/gtk.h>

#include <gtk-layer-shell.h>

typedef struct PanelTrayMenu {
    GtkBox *box;

    GtkWindow *window;

    bool visible;

    gpointer panel_ptr;

    gint x, y;
} PanelTrayMenu;

PanelTrayMenu *panel_tray_menu_new (gpointer panel_ptr);

void *panel_tray_menu_toggle_show (PanelTrayMenu *self);