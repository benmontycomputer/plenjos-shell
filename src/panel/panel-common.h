#pragma once

#include "taskbar/panel-taskbar.h"

#include <gtk/gtk.h>

typedef struct {
    PanelTaskbar *taskbar;

    GdkPixbuf *blurred;

    GtkWindow *gtk_window;

    bool supports_alpha;
    bool dark_mode;
} Panel;

#define UNUSED(x) (void)(x)

#define DATA_DIRS_DEFAULT "/usr/share:/usr/local/share"