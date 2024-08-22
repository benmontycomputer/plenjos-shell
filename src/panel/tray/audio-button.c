#include "audio-button.h"

static void
show_audio_menu (GtkButton *button, AudioButton *self) {
    UNUSED (button);

    gtk_stack_set_visible_child_name (self->stack, "Sound");
}

// https://stackoverflow.com/questions/63527700/alsa-in-c-making-the-minimal-working-code
static void
SetAlsaMasterVolume (long volume, AudioButton *self) {
    long min, max;

    snd_mixer_selem_get_playback_volume_range (self->elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all (self->elem, volume * max / 100);
}

static void
value_changed (GtkRange *volume, AudioButton *self) {
    SetAlsaMasterVolume ((long)gtk_range_get_value (volume), self);
}

static void
destroy (GtkWidget *widget, AudioButton *self) {
    UNUSED (widget);

    snd_mixer_close (self->handle);
}

AudioButton *
audio_button_new (GtkStack *stack) {
    AudioButton *self = malloc (sizeof (AudioButton));

    self->box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

    g_signal_connect (self->box, "destroy", G_CALLBACK (destroy), self);

    self->button = GTK_BUTTON (gtk_button_new_from_icon_name (
        "audio-volume-medium-symbolic", GTK_ICON_SIZE_DND));

    gtk_widget_set_name (GTK_WIDGET (self->button), "panel_tray_menu_button");

    g_signal_connect (self->button, "clicked", G_CALLBACK (show_audio_menu),
                      self);

    GtkAdjustment *adjustment = gtk_adjustment_new (50, 0, 100, 1, 1, 0);

    self->volume
        = GTK_SCALE (gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, adjustment));

    gtk_scale_set_digits (self->volume, 0);

    g_signal_connect (self->volume, "value-changed",
                      G_CALLBACK (value_changed), self);

    gtk_widget_set_size_request (GTK_WIDGET (self->volume), 240, -1);

    gtk_box_pack_start (self->box, GTK_WIDGET (self->volume), FALSE, FALSE, 0);

    gtk_widget_show_all (GTK_WIDGET (self->box));

    gtk_stack_add_titled (stack, GTK_WIDGET (self->box), "Sound", "Sound");

    const char *card = "default";
    const char *selem_name = "Master";

    snd_mixer_open (&self->handle, 0);
    snd_mixer_attach (self->handle, card);
    snd_mixer_selem_register (self->handle, NULL, NULL);
    snd_mixer_load (self->handle);

    snd_mixer_selem_id_alloca (&self->sid);
    snd_mixer_selem_id_set_index (self->sid, 0);
    snd_mixer_selem_id_set_name (self->sid, selem_name);

    self->elem = snd_mixer_find_selem (self->handle, self->sid);

    long min, max;

    snd_mixer_selem_get_playback_volume_range (self->elem, &min, &max);

    long vol = 0;
    snd_mixer_selem_get_playback_volume (self->elem, 0, &vol);

    gtk_range_set_value (GTK_RANGE (self->volume),
                         round(((double)vol * 100.0) / (double)max));

    self->stack = stack;

    return self;
}