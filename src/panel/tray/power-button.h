#include <gtk/gtk.h>
#include <glib.h>

#include "panel-tray-menu.h"

typedef struct PowerButton {
    GtkButton *button;

    PanelTrayMenu *menu;

    GtkButton *shutdown;
    GtkButton *restart;
    GtkButton *lock;
    GtkButton *logout;
} PowerButton;

PowerButton *power_button_new (gpointer panel_ptr);