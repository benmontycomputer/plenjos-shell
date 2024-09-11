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