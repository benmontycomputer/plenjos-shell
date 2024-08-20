#include "panel-tray-menu.h"

gboolean expose_draw_tray_menu (GtkWidget *widget, cairo_t *cr,
                                PanelTrayMenu *self);

static gboolean
check_escape (GtkWidget *widget, GdkEventKey *event, PanelTrayMenu *self) {
    UNUSED (widget);

    if (self->visible && event->keyval == GDK_KEY_Escape) {
        panel_tray_menu_toggle_show (self);
        return TRUE;
    }

    return FALSE;
}

static gboolean
focus_out_event (GtkWidget *widget, GdkEventFocus *event,
                 PanelTrayMenu *self) {
    UNUSED (widget);
    UNUSED (event);

    if (self->visible)
        panel_tray_menu_toggle_show (self);

    return FALSE;
}

static gboolean
configure_event (GtkWidget *win, GdkEventConfigure *event,
                 PanelTrayMenu *self) {
    UNUSED (win);

    GdkWindow *gdk_window = gtk_widget_get_window (GTK_WIDGET (self->window));

    GdkRectangle geo;

    gdk_monitor_get_geometry (
        gdk_display_get_monitor_at_window (gdk_window_get_display (gdk_window),
                                           gdk_window),
        &geo);

    self->x = geo.width - 4 - event->width;
    self->y = geo.height - event->height;

    return FALSE;
}

PanelTrayMenu *
panel_tray_menu_new (gpointer panel_ptr) {
    PanelTrayMenu *self = malloc (sizeof (PanelTrayMenu));

    self->panel_ptr = panel_ptr;

    self->visible = FALSE;

    self->x = self->y = 0;

    self->window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));

    g_signal_connect (self->window, "configure-event",
                      G_CALLBACK (configure_event), self);

    // Before the window is first realized, set it up to be a layer surface
    gtk_layer_init_for_window (self->window);

    // Order below normal windows
    gtk_layer_set_layer (self->window, GTK_LAYER_SHELL_LAYER_OVERLAY);

    //gtk_layer_set_keyboard_mode (
    //    self->window,
    //    GTK_LAYER_SHELL_KEYBOARD_MODE_ON_DEMAND); // NONE is default

    gtk_layer_set_anchor (self->window, GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
    gtk_layer_set_anchor (self->window, GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);

    gtk_widget_set_size_request (GTK_WIDGET (self->window), 400, -1);

    gtk_widget_set_app_paintable (GTK_WIDGET (self->window), TRUE);

    g_signal_connect (self->window, "draw", G_CALLBACK (expose_draw_tray_menu),
                      self);

    //g_signal_connect (self->window, "key_press_event",
    //                  G_CALLBACK (check_escape), self);
    //g_signal_connect (self->window, "focus-out-event",
    //                  G_CALLBACK (focus_out_event), self);

    self->box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

    gtk_container_add (GTK_CONTAINER (self->window), GTK_WIDGET (self->box));

    gtk_widget_set_name (GTK_WIDGET (self->window), "panel_tray_menu_window");
    gtk_widget_set_name (GTK_WIDGET (self->box), "panel_tray_menu_window_box");

    return self;
}

void
panel_tray_menu_toggle_show (PanelTrayMenu *self) {
    if (self->visible) {
        gtk_widget_hide (GTK_WIDGET (self->window));
    } else {
        gtk_widget_show (GTK_WIDGET (self->window));

        gtk_layer_set_margin (self->window, GTK_LAYER_SHELL_EDGE_BOTTOM, 0);
        gtk_layer_set_margin (self->window, GTK_LAYER_SHELL_EDGE_RIGHT, 4);

        GdkWindow *gdk_window
            = gtk_widget_get_window (GTK_WIDGET (self->window));

        GdkRectangle geo;

        gdk_monitor_get_geometry (
            gdk_display_get_monitor_at_window (
                gdk_window_get_display (gdk_window), gdk_window),
            &geo);

        self->x = geo.width - 4 - gdk_window_get_width (gdk_window);
        self->y = geo.height - gdk_window_get_height (gdk_window);
    }

    self->visible = !self->visible;
}