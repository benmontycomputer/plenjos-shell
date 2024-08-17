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

GtkWidget *generate_player (PlayerctlPlayer *player) {
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

    gtk_grid_attach (grid, GTK_WIDGET (label), 1, 0, 1, 1);

    gtk_container_set_focus_chain (GTK_CONTAINER (grid), NULL);

    gtk_widget_show_all (grid);

    return grid;
}

void on_name_appeared (PlayerctlPlayerManager *manager, PlayerctlPlayerName *name, MediaControl *self) {
    PlayerctlPlayer *player = playerctl_player_new_from_name (name, NULL);

    GtkWidget *player_gtk = generate_player (player);

    gtk_box_pack_start (self->box, player_gtk, FALSE, FALSE, 0);

    gtk_widget_show_all (self->box);
}

void on_player_vanished (PlayerctlPlayerManager *manager, PlayerctlPlayer *player, MediaControl *self) {
    
}

void add_name (PlayerctlPlayerName *name, MediaControl *self) {
    on_name_appeared (NULL, name, self);
}

MediaControl *media_control_new () {
    MediaControl *self = malloc (sizeof (MediaControl));

    self->manager = playerctl_player_manager_new (NULL);

    g_signal_connect (self->manager, "name-appeared", on_name_appeared, self);
    g_signal_connect (self->manager, "player-vanished", on_player_vanished, self);

    self->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    GList *names = playerctl_list_players (NULL);

    g_list_foreach (names, add_name, self);

    return self;
}