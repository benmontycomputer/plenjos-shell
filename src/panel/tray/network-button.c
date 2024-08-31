#include "network-button.h"

static void
show_network_menu (GtkButton *button, NetworkButton *self) {
    UNUSED (button);

    gtk_stack_set_visible_child_name (self->stack, "Network");
}

NetworkButton *
network_button_new (GtkStack *stack) {
    NetworkButton *self = malloc (sizeof (NetworkButton));

    self->box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

    self->button = GTK_BUTTON (
        gtk_button_new_from_icon_name ("network-wired-symbolic"));

    gtk_widget_set_size_request (GTK_WIDGET (self->button), 32, 32);

    gtk_widget_set_name (GTK_WIDGET (self->button), "panel_tray_menu_button");

    g_signal_connect (self->button, "clicked", G_CALLBACK (show_network_menu),
                      self);

    gtk_stack_add_titled (stack, GTK_WIDGET (self->box), "Network", "Network");

    self->stack = stack;

    return self;
}