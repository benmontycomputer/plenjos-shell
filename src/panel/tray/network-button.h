#include <gtk/gtk.h>

typedef struct NetworkButton {
    GtkStack *stack;

    GtkBox *box;

    GtkButton *button;
} NetworkButton;

NetworkButton *network_button_new (GtkStack *stack);