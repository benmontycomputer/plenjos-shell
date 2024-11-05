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
        gtk_button_new ());

    GtkBox *button_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8));

    GtkWidget *img = gtk_image_new_from_icon_name ("network-idle");
    gtk_widget_add_css_class (img, "control_center_button_image");
    gtk_widget_set_size_request (img, 36, 36);

    GtkBox *labels = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

    GtkWidget *network_label = gtk_label_new ("Network");
    GtkWidget *status_label = gtk_label_new ("Unknown");

    gtk_widget_set_halign (network_label, GTK_ALIGN_START);
    gtk_widget_set_halign (status_label, GTK_ALIGN_START);

    gtk_box_append (labels, network_label);
    gtk_box_append (labels, status_label);

    gtk_widget_add_css_class (GTK_WIDGET (labels), "control_center_button_labels_box");

    gtk_box_append (button_box, img);
    gtk_box_append (button_box, GTK_WIDGET (labels));

    gtk_button_set_child (self->button, GTK_WIDGET (button_box));

    gtk_widget_set_size_request (GTK_WIDGET (self->button), 36, 36);

    gtk_widget_add_css_class (GTK_WIDGET (self->button), "control_center_button");

    g_signal_connect (self->button, "clicked", G_CALLBACK (show_network_menu),
                      self);

    gtk_stack_add_titled (stack, GTK_WIDGET (self->box), "Network", "Network");

    self->stack = stack;

    return self;
}