#include <gtk/gtk.h>
#include <glib.h>

typedef struct {
    GtkLabel *label;

    time_t rawtime;
    struct tm *timeinfo;

    char text[256];

    char min[3];
    char sec[3];
} PanelClock;

PanelClock *panel_clock_new ();

gboolean panel_clock_update (PanelClock *self);