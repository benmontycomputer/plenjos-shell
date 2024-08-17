#include <gtk/gtk.h>
#include <glib.h>

typedef struct PowerButton {
    GtkButton *button;
} PowerButton;

PowerButton *power_button_new ();