#include "panel-tray-menu.h"

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

PanelTrayMenu *
panel_tray_menu_new (Panel *panel) {
    PanelTrayMenu *self = malloc (sizeof (PanelTrayMenu));

    self->panel = panel;

    self->visible = FALSE;

    self->window = GTK_WINDOW (gtk_window_new ());

    // Before the window is first realized, set it up to be a layer surface
    gtk_layer_init_for_window (self->window);

    // Order below normal windows
    gtk_layer_set_layer (self->window, GTK_LAYER_SHELL_LAYER_OVERLAY);

    gtk_layer_set_keyboard_mode (
        self->window,
        GTK_LAYER_SHELL_KEYBOARD_MODE_ON_DEMAND); // NONE is default

    gtk_layer_set_anchor (self->window, GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
    gtk_layer_set_anchor (self->window, GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);

    gtk_widget_set_size_request (GTK_WIDGET (self->window), 400, -1);

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

    return self;
}

void
panel_tray_menu_toggle_show (PanelTrayMenu *self) {
    if (self->visible) {
        gtk_widget_hide (GTK_WIDGET (self->window));
    } else {
        gtk_widget_show (GTK_WIDGET (self->window));
    }

    self->visible = !self->visible;
}