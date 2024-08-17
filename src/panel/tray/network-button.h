#include <gtk/gtk.h>

typedef struct NetworkButton {
    GtkButton *button;
} NetworkButton;

NetworkButton *network_button_new ();