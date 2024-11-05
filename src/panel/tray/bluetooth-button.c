#include "bluetooth-button.h"

static void
show_bluetooth_menu (GtkButton *button, BluetoothButton *self) {
    UNUSED (button);

    gtk_stack_set_visible_child_name (self->stack, "Bluetooth");
}

BluetoothButton *
bluetooth_button_new (GtkStack *stack) {
    BluetoothButton *self = malloc (sizeof (BluetoothButton));

    self->box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

    self->settings = BLUETOOTH_SETTINGS_WIDGET (bluetooth_settings_widget_new ());
    gtk_widget_set_hexpand (GTK_WIDGET (self->settings), FALSE);

    gtk_widget_add_css_class (GTK_WIDGET (self->settings), "panel_tray_bluetooth_settings");

    gtk_box_append (self->box, GTK_WIDGET (self->settings));

    self->button = GTK_BUTTON (
        gtk_button_new ());

    GtkBox *button_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8));

    GtkWidget *img = gtk_image_new_from_icon_name ("bluetooth-active-symbolic");
    gtk_widget_add_css_class (img, "control_center_button_image");
    gtk_widget_set_size_request (img, 36, 36);

    GtkBox *labels = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

    GtkWidget *bluetooth_label = gtk_label_new ("Bluetooth");
    GtkWidget *status_label = gtk_label_new ("Unknown");

    gtk_widget_set_halign (bluetooth_label, GTK_ALIGN_START);
    gtk_widget_set_halign (status_label, GTK_ALIGN_START);

    gtk_box_append (labels, bluetooth_label);
    gtk_box_append (labels, status_label);

    gtk_widget_add_css_class (GTK_WIDGET (labels), "control_center_button_labels_box");

    gtk_box_append (button_box, img);
    gtk_box_append (button_box, GTK_WIDGET (labels));

    gtk_button_set_child (self->button, GTK_WIDGET (button_box));

    gtk_widget_set_size_request (GTK_WIDGET (self->button), 36, 36);

    gtk_widget_add_css_class (GTK_WIDGET (self->button), "control_center_button");

    g_signal_connect (self->button, "clicked", G_CALLBACK (show_bluetooth_menu),
                      self);

    gtk_stack_add_titled (stack, GTK_WIDGET (self->box), "Bluetooth", "Bluetooth");

    self->stack = stack;

    return self;
}