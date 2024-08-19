#include "media-control.h"

void
previous (GtkButton *self, PlayerctlPlayer *player) {
    playerctl_player_previous (player, NULL);
}

void
play_pause (GtkButton *self, PlayerctlPlayer *player) {
    playerctl_player_play_pause (player, NULL);
}

void
next (GtkButton *self, PlayerctlPlayer *player) {
    playerctl_player_next (player, NULL);
}

// "<song> - <artist> (<album>)"

void
on_metadata (PlayerctlPlayer *player, GVariant *metadata, GtkLabel *label) {
    char *title
        = playerctl_player_print_metadata_prop (player, "xesam:title", NULL);
    char *artist
        = playerctl_player_print_metadata_prop (player, "xesam:artist", NULL);
    char *album
        = playerctl_player_print_metadata_prop (player, "xesam:album", NULL);
    char *url
        = playerctl_player_print_metadata_prop (player, "xesam:url", NULL);
    // char *track = playerctl_player_print_metadata_prop (player,
    // "mpris:trackid", NULL);

    /* if (title == NULL) {
        title = playerctl_player_get_title (player, NULL);
    }

    if (artist == NULL) {
        artist = playerctl_player_get_artist (player, NULL);
    }

    if (album == NULL) {
        album = playerctl_player_get_album (player, NULL);
    } */

    if (title == NULL) {
        title = url;
    }

    if (title == NULL) {
        title = "(unknown)";
    }

    if (artist == NULL) {
        if (album == NULL) {
            size_t formatted_len = strlen (title) + 1;
            char *formatted = malloc (formatted_len);

            snprintf (formatted, formatted_len, "%s", title);

            gtk_label_set_text (label, formatted);

            free (formatted);
        }
    } else if (album == NULL) {
        size_t formatted_len
            = strlen (title) + strlen (artist) + strlen (" - ") + 1;
        char *formatted = malloc (formatted_len);

        snprintf (formatted, formatted_len, "%s - %s", title, artist);

        gtk_label_set_text (label, formatted);

        free (formatted);
    } else {
        size_t formatted_len = strlen (title) + strlen (artist)
                               + strlen (album) + strlen (" -  ()") + 1;
        char *formatted = malloc (formatted_len);

        snprintf (formatted, formatted_len, "%s - %s (%s)", title, artist,
                  album);

        gtk_label_set_text (label, formatted);

        free (formatted);
    }

    if (url) {
        if (title == url) {
            title = NULL;
        }

        free (url);

        url = NULL;
    }

    if (title) {
        free (title);

        title = NULL;
    }

    if (artist) {
        free (artist);

        artist = NULL;
    }

    if (album) {
        free (album);

        album = NULL;
    }
}

typedef struct GtkPlayer {
    GtkWidget *widget;

    MediaControl *self;

    PlayerctlPlayer *player;
} GtkPlayer;

GtkPlayer *
generate_player (PlayerctlPlayerName *name, MediaControl *self) {
    PlayerctlPlayer *player = playerctl_player_new_from_name (name, NULL);

    playerctl_player_manager_manage_player (self->manager, player);

    GtkBox *box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 2));

    gtk_widget_set_name (GTK_WIDGET (box), "media_player_box");
    gtk_widget_set_hexpand (GTK_WIDGET (box), TRUE);
    gtk_widget_set_halign (GTK_WIDGET (box), GTK_ALIGN_FILL);

    GtkButton *previous_button = gtk_button_new_from_icon_name (
        "media-skip-backward", GTK_ICON_SIZE_DND);
    GtkButton *play_button = gtk_button_new_from_icon_name (
        "media-playback-start", GTK_ICON_SIZE_DND);
    GtkButton *next_button = gtk_button_new_from_icon_name (
        "media-skip-forward", GTK_ICON_SIZE_DND);

    gtk_widget_set_name (GTK_WIDGET (previous_button), "media_player_button");
    gtk_widget_set_name (GTK_WIDGET (play_button), "media_player_button");
    gtk_widget_set_name (GTK_WIDGET (next_button), "media_player_button");

    g_signal_connect (previous_button, "clicked", previous, player);
    g_signal_connect (play_button, "clicked", play_pause, player);
    g_signal_connect (next_button, "clicked", next, player);

    GtkBox *controls_box
        = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2));

    gtk_widget_set_halign (GTK_WIDGET (controls_box), GTK_ALIGN_CENTER);

    gtk_container_add (GTK_CONTAINER (controls_box),
                       GTK_WIDGET (previous_button));
    gtk_container_add (GTK_CONTAINER (controls_box), GTK_WIDGET (play_button));
    gtk_container_add (GTK_CONTAINER (controls_box), GTK_WIDGET (next_button));

    GtkLabel *label = GTK_LABEL (gtk_label_new ("Loading..."));

    gtk_widget_set_hexpand (label, TRUE);
    gtk_label_set_max_width_chars (label, 0);
    gtk_label_set_line_wrap (label, TRUE);
    gtk_label_set_line_wrap_mode (label, PANGO_WRAP_WORD_CHAR);

    GtkImage *icon = GTK_IMAGE (gtk_image_new_from_icon_name (name->name, GTK_ICON_SIZE_DIALOG));

    gtk_widget_set_halign (GTK_WIDGET (icon), GTK_ALIGN_START);

    GtkBox *info_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);

    gtk_container_add (GTK_CONTAINER (info_box), GTK_WIDGET (icon));
    gtk_container_add (GTK_CONTAINER (info_box), GTK_WIDGET (label));

    gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (info_box));
    gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (controls_box));

    gtk_widget_show_all (box);

    g_signal_connect (player, "metadata", on_metadata, label);

    on_metadata (player, NULL, label);

    GtkPlayer *return_val = malloc (sizeof (GtkPlayer *));

    return_val->player = player;
    return_val->widget = box;
    return_val->self = self;

    return return_val;
}

void
on_name_appeared (PlayerctlPlayerManager *manager, PlayerctlPlayerName *name,
                  MediaControl *self) {
    GtkPlayer *player_gtk = generate_player (name, self);

    gtk_box_pack_start (self->box, player_gtk->widget, FALSE, FALSE, 0);

    self->gtk_players = g_list_append (self->gtk_players, player_gtk);

    gtk_widget_show_all (self->box);
}

void
player_vanished_foreach (GtkPlayer *player, PlayerctlPlayer *remove_player) {
    if (player->player == remove_player) {
        gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (player->widget)),
                              player->widget);

        g_object_unref (remove_player);

        player->self->gtk_players = g_list_remove (player->self->gtk_players, player);

        free (player);
    }
}

void
on_player_vanished (PlayerctlPlayerManager *manager, PlayerctlPlayer *player,
                    MediaControl *self) {
    g_list_foreach (self->gtk_players, player_vanished_foreach, player);
}

void
add_name (PlayerctlPlayerName *name, MediaControl *self) {
    on_name_appeared (NULL, name, self);
}

MediaControl *
media_control_new () {
    MediaControl *self = malloc (sizeof (MediaControl));

    self->gtk_players = NULL;

    self->manager = playerctl_player_manager_new (NULL);

    g_signal_connect (self->manager, "name-appeared", on_name_appeared, self);
    g_signal_connect (self->manager, "player-vanished", on_player_vanished,
                      self);

    self->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    GList *names = playerctl_list_players (NULL);

    g_list_foreach (names, add_name, self);

    return self;
}