#include "panel-tray-menu.h"

static gboolean
event_handle (GdkSurface *window, GdkEvent *event, PanelTrayMenu *self) {
    GdkEventType type = gdk_event_get_event_type (event);

    if (type == GDK_BUTTON_PRESS) {
        printf("test\n\n");
        fflush(stdout);
    }

    return FALSE;
}

static gboolean
check_escape (GtkEventControllerKey *key_controller, guint keyval,
              guint keycode, GdkModifierType state, PanelTrayMenu *self) {
    UNUSED (key_controller);
    UNUSED (keycode);
    UNUSED (state);

    if (self->visible && keyval == GDK_KEY_Escape) {
        panel_tray_menu_toggle_show (self);
        return TRUE;
    }

    return FALSE;
}

static void
focus_out_event (GtkEventControllerFocus *focus_controller,
                 PanelTrayMenu *self) {
    UNUSED (focus_controller);

    if (self->visible)
        panel_tray_menu_toggle_show (self);

    return;
}

static void realize (GtkWidget *widget, PanelTrayMenu *self) {
    GdkSurface *surface = gtk_native_get_surface (GTK_NATIVE (self->window));

    g_signal_connect (surface, "event", G_CALLBACK (event_handle), self);
}

PanelTrayMenu *
panel_tray_menu_new (Panel *panel) {
    PanelTrayMenu *self = malloc (sizeof (PanelTrayMenu));

    self->panel = panel;

    self->visible = FALSE;

    self->window = GTK_WINDOW (gtk_window_new ());

    gtk_layer_init_for_window (self->window);

    gtk_layer_set_layer (self->window, GTK_LAYER_SHELL_LAYER_OVERLAY);

    gtk_layer_set_keyboard_mode (self->window,
                                 GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);

    gtk_layer_set_margin (self->window, GTK_LAYER_SHELL_EDGE_TOP, 8);
    gtk_layer_set_margin (self->window, GTK_LAYER_SHELL_EDGE_RIGHT, 8);

    static const gboolean anchors[] = { FALSE, TRUE, TRUE, FALSE };
    for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++) {
        gtk_layer_set_anchor (self->window, i, anchors[i]);
    }

    gtk_layer_set_exclusive_zone (self->window, 0);

    GtkEventController *key_controller = gtk_event_controller_key_new ();
    GtkEventController *focus_controller = gtk_event_controller_focus_new ();

    gtk_widget_add_controller (GTK_WIDGET (self->window), key_controller);
    gtk_widget_add_controller (GTK_WIDGET (self->window), focus_controller);

    g_signal_connect (key_controller, "key-pressed", G_CALLBACK (check_escape),
                      self);
    g_signal_connect (focus_controller, "leave", G_CALLBACK (focus_out_event),
                      self);

    self->box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

    gtk_window_set_child (self->window, GTK_WIDGET (self->box));

    gtk_widget_set_name (GTK_WIDGET (self->window), "panel_tray_menu_window");
    gtk_widget_set_name (GTK_WIDGET (self->box), "panel_tray_menu_window_box");

    g_signal_connect (self->window, "realize", G_CALLBACK (realize), self);

    gtk_window_present (self->window);

    gtk_widget_hide (GTK_WIDGET (self->window));

    return self;
}

void
panel_tray_menu_toggle_show (PanelTrayMenu *self) {
    if (self->visible) {
        gtk_widget_hide (GTK_WIDGET (self->window));
    } else {
        gtk_widget_grab_focus (GTK_WIDGET (self->window));
        gtk_widget_show (GTK_WIDGET (self->window));
    }

    self->visible = !self->visible;
}