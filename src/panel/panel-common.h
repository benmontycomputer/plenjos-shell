#pragma once

#include "taskbar/panel-taskbar.h"

#include <gtk/gtk.h>

enum PanelStyle {
    PANEL_STYLE_THREE_D_DOCK,
    PANEL_STYLE_DOCK,
    PANEL_STYLE_PANEL,
    PANEL_STYLE_INVISIBLE
};

typedef struct {
    PanelTaskbar *taskbar;
    gpointer *tray;

    GdkPixbuf *blurred;

    GtkWindow *gtk_window;

    bool supports_alpha;
    bool dark_mode;

    GSettings *panel_settings;

    enum PanelStyle style;

    GdkRGBA bg_primary;
    GdkRGBA bg_primary_bottom;
    GdkRGBA bg_secondary;
    GdkRGBA border_primary;
    GdkRGBA border_secondary;

    GtkApplication *app;

    GListModel *monitors;
} Panel;

#define UNUSED(x) (void)(x)

#define DATA_DIRS_DEFAULT "/usr/share:/usr/local/share"