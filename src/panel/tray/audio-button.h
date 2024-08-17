#include <gtk/gtk.h>

typedef struct AudioButton {
    GtkButton *button;
} AudioButton;

AudioButton *audio_button_new ();