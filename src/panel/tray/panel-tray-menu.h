#include <stdio.h>
#include <stdbool.h>

#include <gtk/gtk.h>

#include <gtk-layer-shell.h>

typedef struct PanelTrayMenu {
    GtkBox *box;

    GtkWindow *window;

    bool visible;
} PanelTrayMenu;

PanelTrayMenu *panel_tray_menu_new ();

void *panel_tray_menu_toggle_show (PanelTrayMenu *self, gint bottom, gint right);