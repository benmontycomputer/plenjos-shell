#pragma once

#include <gtk/gtk.h>
#include <glib.h>

typedef struct Clock {
    GtkLabel *label;

    time_t rawtime;
    struct tm *timeinfo;

    char text[256];

    char min[3];
    char sec[3];
} Clock;

Clock *clock_new ();

gboolean clock_update (Clock *self);