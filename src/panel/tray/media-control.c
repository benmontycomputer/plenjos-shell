#include "media-control.h"

void previous (GtkButton *self, PlayerctlPlayer *player) {
    playerctl_player_previous (player, NULL);
}

void play_pause (GtkButton *self, PlayerctlPlayer *player) {
    playerctl_player_play_pause (player, NULL);
}

void next (GtkButton *self, PlayerctlPlayer *player) {
    playerctl_player_next (player, NULL);
}

// "<song> - <artist> (<album>)"

void on_metadata (PlayerctlPlayer *player, GVariant *metadata, GtkLabel *label) {
    char *title = playerctl_player_get_title (player, NULL);
    char *artist = playerctl_player_get_artist (player, NULL);
    char *album = playerctl_player_get_album (player, NULL);

    printf("test\n");
    fflush(stdout);

    if (title == NULL) {
        title = "(unknown)";
    }

    if (artist == NULL) {
        if (album == NULL) {
            size_t formatted_len = strlen (title) + 1;
            char *formatted = malloc (formatted_len);

            snprintf (formatted, formatted_len, "%s", title);

            gtk_label_set_text (label, formatted);
        }
    } else if (album == NULL) {
        size_t formatted_len = strlen (title) + strlen (artist) + strlen (" - ") + 1;
        char *formatted = malloc (formatted_len);

        snprintf (formatted, formatted_len, "%s - %s", title, artist);

        gtk_label_set_text (label, formatted);
    } else {
        size_t formatted_len = strlen (title) + strlen (artist) + strlen (album) + strlen (" -  ()") + 1;
        char *formatted = malloc (formatted_len);

        snprintf (formatted, formatted_len, "%s - %s (%s)", title, artist, album);

        gtk_label_set_text (label, formatted);
    }
}

typedef struct GtkPlayer {
    GtkWidget *widget;

    MediaControl *self;

    PlayerctlPlayer *player;
} GtkPlayer;

GtkPlayer *generate_player (PlayerctlPlayer *player) {
    GtkGrid *grid = GTK_GRID (gtk_grid_new ());

    gtk_grid_set_column_homogeneous (grid, TRUE);

    gtk_widget_set_name (GTK_WIDGET (grid), "media_player_grid");
    gtk_widget_set_hexpand (GTK_WIDGET (grid), FALSE);
    gtk_widget_set_halign (GTK_WIDGET (grid), GTK_ALIGN_CENTER);

    GtkButton *previous_button = gtk_button_new_from_icon_name ("media-skip-backward", GTK_ICON_SIZE_DND);
    GtkButton *play_button = gtk_button_new_from_icon_name ("media-playback-start", GTK_ICON_SIZE_DND);
    GtkButton *next_button = gtk_button_new_from_icon_name ("media-skip-forward", GTK_ICON_SIZE_DND);

    gtk_widget_set_name (GTK_WIDGET (previous_button), "media_player_button");
    gtk_widget_set_name (GTK_WIDGET (play_button), "media_player_button");
    gtk_widget_set_name (GTK_WIDGET (next_button), "media_player_button");


    g_signal_connect (previous_button, "clicked", previous, player);
    g_signal_connect (play_button, "clicked", play_pause, player);
    g_signal_connect (next_button, "clicked", next, player);

    gtk_grid_attach (grid, GTK_WIDGET (previous_button), 0, 1, 1, 1);
    gtk_grid_attach (grid, GTK_WIDGET (play_button), 1, 1, 1, 1);
    gtk_grid_attach (grid, GTK_WIDGET (next_button), 2, 1, 1, 1);

    GtkLabel *label = GTK_LABEL (gtk_label_new ("Loading..."));

    gtk_grid_attach (grid, GTK_WIDGET (label), 0, 0, 3, 1);

    gtk_container_set_focus_chain (GTK_CONTAINER (grid), NULL);

    gtk_widget_show_all (grid);

    g_signal_connect (player, "metadata", on_metadata, label);

    on_metadata (player, NULL, label);

    GtkPlayer *return_val = malloc (sizeof (GtkPlayer *));

    return_val->player = player;
    return_val->widget = grid;

    return return_val;
}

void on_name_appeared (PlayerctlPlayerManager *manager, PlayerctlPlayerName *name, MediaControl *self) {
    PlayerctlPlayer *player = playerctl_player_new_from_name (name, NULL);

    playerctl_player_manager_manage_player (self->manager, player);

    GtkPlayer *player_gtk = generate_player (player);

    gtk_box_pack_start (self->box, player_gtk->widget, FALSE, FALSE, 0);

    self->gtk_players = g_list_append (self->gtk_players, player_gtk);

    gtk_widget_show_all (self->box);
}

void player_vanished_foreach (GtkPlayer *player, PlayerctlPlayer *remove_player) {
    if (player->player == remove_player) {
        gtk_container_remove (gtk_widget_get_parent (player->widget), player->widget);
        g_object_unref (player->widget);
    }
}

void on_player_vanished (PlayerctlPlayerManager *manager, PlayerctlPlayer *player, MediaControl *self) {
    g_list_foreach (self->gtk_players, player_vanished_foreach, player);
}

void add_name (PlayerctlPlayerName *name, MediaControl *self) {
    on_name_appeared (NULL, name, self);
}

MediaControl *media_control_new () {
    MediaControl *self = malloc (sizeof (MediaControl));

    self->gtk_players = NULL;

    self->manager = playerctl_player_manager_new (NULL);

    g_signal_connect (self->manager, "name-appeared", on_name_appeared, self);
    g_signal_connect (self->manager, "player-vanished", on_player_vanished, self);

    self->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    GList *names = playerctl_list_players (NULL);

    g_list_foreach (names, add_name, self);

    return self;
}