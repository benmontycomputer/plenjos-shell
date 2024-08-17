#include "panel-tray-menu.h"

PanelTrayMenu *panel_tray_menu_new () {
    PanelTrayMenu *self = malloc (sizeof (PanelTrayMenu));

    self->visible = FALSE;

    self->window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));

    // Before the window is first realized, set it up to be a layer surface
    gtk_layer_init_for_window (self->window);

    // Order below normal windows
    gtk_layer_set_layer (self->window, GTK_LAYER_SHELL_LAYER_TOP);

    gtk_layer_set_anchor (self->window, GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
    gtk_layer_set_anchor (self->window, GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);

    gtk_widget_set_size_request (GTK_WIDGET (self->window), 480, 640);

    self->box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

    gtk_container_add (GTK_CONTAINER (self->window), GTK_WIDGET (self->box));

    return self;
}

void *panel_tray_menu_toggle_show (PanelTrayMenu *self, gint bottom, gint right) {
    if (self->visible) {
        gtk_widget_hide (GTK_WIDGET (self->window));
    } else {
        gtk_widget_show (GTK_WIDGET (self->window));

        gtk_layer_set_margin (self->window, GTK_LAYER_SHELL_EDGE_BOTTOM, bottom);
        gtk_layer_set_margin (self->window, GTK_LAYER_SHELL_EDGE_RIGHT, right);
    }

    self->visible = !self->visible;
}