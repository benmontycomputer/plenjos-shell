#include <gtk/gtk.h>
#include <glib.h>

#include "panel-tray-menu.h"

typedef struct PowerButton {
    GtkButton *button;

    PanelTrayMenu *menu;
} PowerButton;

PowerButton *power_button_new ();