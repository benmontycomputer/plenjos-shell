#include "audio-button.h"

void show_audio_menu (GtkButton *button, AudioButton *self) {
    gtk_stack_set_visible_child_name (self->stack, "Sound");
}

AudioButton *
audio_button_new (GtkStack *stack) {
    AudioButton *self = malloc (sizeof (AudioButton));
    
    self->box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

    self->button = GTK_BUTTON (gtk_button_new_from_icon_name ("audio-volume-medium-symbolic",
                                                  GTK_ICON_SIZE_DND));

    gtk_widget_set_name (self->button, "panel_tray_menu_button");

    g_signal_connect (self->button, "clicked", show_audio_menu, self);

    GtkAdjustment *adjustment = gtk_adjustment_new (50, 0, 100, 1, 1, 0);

    self->volume = GTK_SCALE (gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, adjustment));

    gtk_scale_set_digits (self->volume, 0);

    gtk_widget_set_size_request (GTK_WIDGET (self->volume), 240, -1);

    gtk_box_pack_start (self->box, GTK_WIDGET (self->volume), FALSE, FALSE, 0);

    gtk_widget_show_all (GTK_WIDGET (self->box));

    gtk_stack_add_titled (stack, GTK_WIDGET (self->box), "Sound", "Sound");

    self->stack = stack;

    return self;
}