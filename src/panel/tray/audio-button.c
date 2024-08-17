#include "audio-button.h"

static void toggle_menu_wrap (GtkButton *button, AudioButton *self) {
    panel_tray_menu_toggle_show (self->menu, 0, 4);
}

AudioButton *
audio_button_new (gpointer panel_ptr) {
    AudioButton *self = malloc (sizeof (AudioButton));

    self->button = gtk_button_new_from_icon_name ("audio-volume-medium",
                                                  GTK_ICON_SIZE_DND);

    gtk_widget_set_name (self->button, "panel_button");

    self->menu = panel_tray_menu_new (panel_ptr);

    g_signal_connect (self->button, "clicked", toggle_menu_wrap, self);

    GtkAdjustment *adjustment = gtk_adjustment_new (50, 0, 100, 1, 1, 0);

    self->volume = GTK_SCALE (gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, adjustment));

    gtk_scale_set_digits (self->volume, 0);

    gtk_widget_set_size_request (GTK_WIDGET (self->volume), 240, -1);

    gtk_box_pack_start (self->menu->box, GTK_WIDGET (self->volume), FALSE, FALSE, 0);

    gtk_widget_show_all (GTK_WIDGET (self->menu->box));

    return self;
}