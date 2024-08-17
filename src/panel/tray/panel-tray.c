#include "panel-tray.h"

PanelTray *
panel_tray_new () {
    PanelTray *self = malloc (sizeof (PanelTray));

    self->audio_button = audio_button_new ();
    self->network_button = network_button_new ();
    self->power_button = power_button_new ();

    self->box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4));

    self->clock = clock_new ();

    g_timeout_add (500, (GSourceFunc)clock_update, self->clock);

    gtk_box_pack_start (self->box, GTK_WIDGET (self->audio_button->button), FALSE,
                        FALSE, 0);
    gtk_box_pack_start (self->box, GTK_WIDGET (self->network_button->button), FALSE,
                        FALSE, 0);
    gtk_box_pack_start (self->box, GTK_WIDGET (self->clock->label), FALSE,
                        FALSE, 0);
    gtk_box_pack_start (self->box, GTK_WIDGET (self->power_button->button), FALSE,
                        FALSE, 0);

    return self;
}