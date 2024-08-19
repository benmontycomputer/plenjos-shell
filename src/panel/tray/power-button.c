#include "power-button.h"

void show_power_menu (GtkButton *button, PowerButton *self) {
    gtk_stack_set_visible_child_name (self->stack, "Power");
}

GtkButton *power_button_menu_button_new (char *icon, char *label) {
    GtkBox *box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8));

    gtk_box_pack_start (box, gtk_image_new_from_icon_name (icon, GTK_ICON_SIZE_DND), FALSE, FALSE, 0);
    gtk_box_pack_start (box, gtk_label_new (label), FALSE, FALSE, 0);

    gtk_widget_show_all (GTK_WIDGET (box));

    GtkButton *button = GTK_BUTTON (gtk_button_new ());

    gtk_widget_set_name (GTK_WIDGET (button), "panel_power_menu_button");

    gtk_container_add (button, GTK_WIDGET (box));

    return button;
}

PowerButton *power_button_new (GtkStack *stack) {
    PowerButton *self = malloc (sizeof (PowerButton));

    self->box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 4));

    self->button = gtk_button_new_from_icon_name ("system-shutdown", GTK_ICON_SIZE_DND);

    gtk_widget_set_name (self->button, "panel_tray_menu_button");

    g_signal_connect (self->button, "clicked", show_power_menu, self);

    self->shutdown = power_button_menu_button_new ("system-shutdown", "Shut down");
    self->restart = power_button_menu_button_new ("system-reboot", "Restart");
    self->lock = power_button_menu_button_new ("system-lock-screen", "Lock");
    self->logout = power_button_menu_button_new ("system-log-out", "Log out");

    gtk_box_pack_start (self->box, GTK_WIDGET (self->shutdown), FALSE, FALSE, 0);
    gtk_box_pack_start (self->box, GTK_WIDGET (self->restart), FALSE, FALSE, 0);
    gtk_box_pack_start (self->box, GTK_WIDGET (self->lock), FALSE, FALSE, 0);
    gtk_box_pack_start (self->box, GTK_WIDGET (self->logout), FALSE, FALSE, 0);

    gtk_widget_show_all (GTK_WIDGET (self->box));

    gtk_stack_add_titled (stack, GTK_WIDGET (self->box), "Power", "Power");

    self->stack = stack;

    return self;
}