#include "panel-taskbar-application.h"
#include "panel-taskbar-toplevel-button.h"

PanelTaskbarApplication *panel_taskbar_application_new (char *id, PanelTaskbar *taskbar) {
    PanelTaskbarApplication *self = malloc (sizeof (PanelTaskbarApplication));

    self->id = g_strdup (id);

    self->toplevels = NULL;

    self->items_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));

    gtk_box_pack_start (taskbar->taskbar_box, GTK_WIDGET (self->items_box), FALSE, FALSE, 0);

    self->taskbar = taskbar;
    taskbar->applications = g_list_append (taskbar->applications, self);

    return self;
}

void panel_taskbar_application_add_toplevel (PanelTaskbarApplication *self, PanelTaskbarToplevelButton *toplevel) {
    if (!g_list_find (self->toplevels, toplevel)) {
        self->toplevels = g_list_append (self->toplevels, toplevel);

        gtk_box_pack_start (self->items_box, toplevel->rendered, FALSE, FALSE, 0);

        gtk_widget_show_all (GTK_WIDGET (self->items_box));
    }
}

void panel_taskbar_application_remove_toplevel (PanelTaskbarApplication *self, PanelTaskbarToplevelButton *toplevel) {
    if (self->toplevels && g_list_find (self->toplevels, toplevel)) {
        gtk_container_remove (GTK_CONTAINER (self->items_box), toplevel->rendered);

        self->toplevels = g_list_remove (self->toplevels, toplevel);
    }

    if (!self->toplevels || !self->toplevels->data) {
        self->taskbar->applications = g_list_remove (self->taskbar->applications, self);
        printf("test\n\n");
        fflush(stdout);

        free (self->id);
        self->id = NULL;
        g_list_free (self->toplevels);
        gtk_container_remove (GTK_CONTAINER (self->taskbar->taskbar_box), GTK_WIDGET (self->items_box));
        free (self);
    }
}