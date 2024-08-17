#include <gtk/gtk.h>

#include "panel-tray-menu.h"

typedef struct AudioButton {
    GtkButton *button;

    PanelTrayMenu *menu;

    GtkScale *volume;
} AudioButton;

AudioButton *audio_button_new (gpointer panel_ptr);