#include <gtk/gtk.h>

#include <playerctl.h>

typedef struct MediaControl {
    PlayerctlPlayerManager *manager;

    GtkBox *box;
} MediaControl;

MediaControl *media_control_new ();