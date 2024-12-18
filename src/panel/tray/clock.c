#include "clock.h"

Clock *clock_new () {
    Clock *self = malloc (sizeof (Clock));

    self->label = GTK_LABEL (gtk_label_new ("Loading..."));
    gtk_label_set_justify (self->label, GTK_JUSTIFY_RIGHT);

    gtk_widget_add_css_class (GTK_WIDGET (self->label), "menu_bar_clock");
    gtk_widget_add_css_class (GTK_WIDGET (self->label), "menu_bar_item");

    return self;
}

gboolean clock_update (Clock *self) {
    time (&self->rawtime);
    self->timeinfo = localtime (&self->rawtime);

    snprintf (self->min, 3, "%d", self->timeinfo->tm_min);

    if (strlen (self->min) == 1) {
        snprintf (self->min, 3, "0%d", self->timeinfo->tm_min);
    }

    snprintf (self->sec, 3, "%d", self->timeinfo->tm_sec);

    if (strlen (self->sec) == 1) {
        snprintf (self->sec, 3, "0%d", self->timeinfo->tm_sec);
    }

    snprintf (self->text, 256, "%d/%d/%d %d:%s:%s", self->timeinfo->tm_mon + 1, self->timeinfo->tm_mday, self->timeinfo->tm_year + 1900, self->timeinfo->tm_hour, self->min, self->sec);

    gtk_label_set_text (self->label, self->text);

    return TRUE;
}