#include <gtk/gtk.h>

typedef struct AudioButton {
    GtkStack *stack;

    GtkBox *box;

    GtkButton *button;

    GtkScale *volume;
} AudioButton;

AudioButton *audio_button_new (GtkStack *stack);