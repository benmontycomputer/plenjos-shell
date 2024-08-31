#include "power-button.h"

static void
show_power_menu (GtkButton *button, PowerButton *self) {
    UNUSED (button);

    gtk_stack_set_visible_child_name (self->stack, "Power");
}

GtkButton *
power_button_menu_button_new (char *icon, char *label) {
    GtkBox *box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8));

    // DND icon size
    gtk_box_append (box, gtk_image_new_from_icon_name (icon));
    gtk_box_append (box, gtk_label_new (label));

    GtkButton *button = GTK_BUTTON (gtk_button_new ());

    gtk_widget_set_name (GTK_WIDGET (button), "panel_power_menu_button");

    gtk_button_set_child (button, GTK_WIDGET (box));

    return button;
}

PowerButton *
power_button_new (GtkStack *stack) {
    PowerButton *self = malloc (sizeof (PowerButton));

    self->box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 4));

    self->button = GTK_BUTTON (
        gtk_button_new_from_icon_name ("system-shutdown-symbolic"));

    gtk_widget_set_size_request (GTK_WIDGET (self->button), 32, 32);

    gtk_widget_set_name (GTK_WIDGET (self->button), "panel_tray_menu_button");

    g_signal_connect (self->button, "clicked", G_CALLBACK (show_power_menu),
                      self);

    self->shutdown
        = power_button_menu_button_new ("system-shutdown", "Shut down");
    self->restart = power_button_menu_button_new ("system-reboot", "Restart");
    self->lock = power_button_menu_button_new ("system-lock-screen", "Lock");
    self->logout = power_button_menu_button_new ("system-log-out", "Log out");

    gtk_box_append (self->box, GTK_WIDGET (self->shutdown));
    gtk_box_append (self->box, GTK_WIDGET (self->restart));
    gtk_box_append (self->box, GTK_WIDGET (self->lock));
    gtk_box_append (self->box, GTK_WIDGET (self->logout));

    gtk_stack_add_titled (stack, GTK_WIDGET (self->box), "Power", "Power");

    self->stack = stack;

    return self;
}