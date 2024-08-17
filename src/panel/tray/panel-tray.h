#include <stdio.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "clock.h"

#include "audio-button.h"
#include "network-button.h"
#include "power-button.h"

typedef struct PanelTray {
    GtkBox *box;

    AudioButton *audio_button;
    NetworkButton *network_button;
    PowerButton *power_button;

    Clock *clock;
} PanelTray;

PanelTray *panel_tray_new (gpointer panel_ptr);