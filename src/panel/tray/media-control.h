#include <gtk/gtk.h>

#include <playerctl.h>

typedef struct MediaControl {
    PlayerctlPlayerManager *manager;

    GtkBox *box;

    GList *gtk_players;
} MediaControl;

MediaControl *media_control_new ();