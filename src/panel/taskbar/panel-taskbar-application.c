#include "panel-taskbar-application.h"

PanelTaskbarApplication *panel_taskbar_application_new (char *id) {
    PanelTaskbarApplication *self = malloc (sizeof (PanelTaskbarApplication));

    self->id = g_strdup (id);

    self->toplevels = g_list_alloc ();

    return self;
}