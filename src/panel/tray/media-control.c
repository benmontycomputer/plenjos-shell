#include "media-control.h"

void
previous (GtkButton *self, PlayerctlPlayer *player) {
    UNUSED (self);

    playerctl_player_previous (player, NULL);
}

void
play_pause (GtkButton *self, PlayerctlPlayer *player) {
    UNUSED (self);

    playerctl_player_play_pause (player, NULL);
}

void
next (GtkButton *self, PlayerctlPlayer *player) {
    UNUSED (self);

    playerctl_player_next (player, NULL);
}

typedef struct GtkPlayer {
    GtkWidget *widget;

    MediaControl *self;

    PlayerctlPlayer *player;

    GtkLabel *label;

    char *bg_path;
    GdkPixbuf *pbuf_blurred;

    gint width;

    GtkButton *play_button;
} GtkPlayer;

static void
gtk_player_update_pbuf (GtkPlayer *gtk_player) {
    if (gtk_player->pbuf_blurred)
        g_object_unref (gtk_player->pbuf_blurred);

    GdkPixbuf *pbuf = NULL;

    if (gtk_player->bg_path && strlen (gtk_player->bg_path) > 7) {
        pbuf = gdk_pixbuf_new_from_file_at_scale (
            gtk_player->bg_path + 7,
            gtk_widget_get_allocated_width (gtk_player->widget) + 96, -1, TRUE,
            NULL);
    }

    /* cairo_surface_t *surfaceold
        = gdk_cairo_surface_create_from_pixbuf (pbuf, 1, NULL);

    cairo_surface_t *surface = cairo_image_surface_create (
        CAIRO_FORMAT_RGB24, cairo_image_surface_get_width (surfaceold),
        cairo_image_surface_get_height (surfaceold));

    stack_blur (surfaceold, surface, 48, 48);

    cairo_t *cr = cairo_create (surface);

    cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 0.5);

    cairo_set_operator (cr, CAIRO_OPERATOR_ATOP);

    cairo_paint (cr);

    cairo_destroy (cr);

    gtk_player->pbuf_blurred = gdk_pixbuf_get_from_surface (
        surface, 48, 0, cairo_image_surface_get_width (surfaceold) - 96,
        cairo_image_surface_get_height (surfaceold));

    g_object_unref (pbuf);
    cairo_surface_destroy (surfaceold);
    cairo_surface_destroy (surface); */

    gtk_player->pbuf_blurred = pbuf;
}

// "<song> - <artist> (<album>)"

static void
on_metadata (PlayerctlPlayer *player, GVariant *metadata,
             GtkPlayer *gtk_player) {
    UNUSED (metadata);

    char *title
        = playerctl_player_print_metadata_prop (player, "xesam:title", NULL);
    char *artist
        = playerctl_player_print_metadata_prop (player, "xesam:artist", NULL);
    char *album
        = playerctl_player_print_metadata_prop (player, "xesam:album", NULL);
    char *url
        = playerctl_player_print_metadata_prop (player, "xesam:url", NULL);
    char *art_url
        = playerctl_player_print_metadata_prop (player, "mpris:artUrl", NULL);

    if (title == NULL) {
        title = url;
    }

    if (title == NULL) {
        title = g_strdup ("(unknown)");
    }

    if (artist == NULL) {
        if (album == NULL) {
            size_t formatted_len = strlen (title) + 1;
            char *formatted = malloc (formatted_len);

            snprintf (formatted, formatted_len, "%s", title);

            gtk_label_set_text (gtk_player->label, formatted);

            free (formatted);
        }
    } else if (album == NULL) {
        size_t formatted_len
            = strlen (title) + strlen (artist) + strlen (" - ") + 1;
        char *formatted = malloc (formatted_len);

        snprintf (formatted, formatted_len, "%s - %s", title, artist);

        gtk_label_set_text (gtk_player->label, formatted);

        free (formatted);
    } else {
        size_t formatted_len = strlen (title) + strlen (artist)
                               + strlen (album) + strlen (" -  ()") + 1;
        char *formatted = malloc (formatted_len);

        snprintf (formatted, formatted_len, "%s - %s (%s)", title, artist,
                  album);

        gtk_label_set_text (gtk_player->label, formatted);

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

    if (art_url) {
        gboolean update_pbuf = FALSE;

        if (gtk_player->bg_path) {
            if (strcmp (art_url, gtk_player->bg_path) != 0) {
                free (gtk_player->bg_path);
                gtk_player->bg_path = g_strdup (art_url);

                update_pbuf = TRUE;
            }
        } else {
            gtk_player->bg_path = g_strdup (art_url);

            update_pbuf = TRUE;
        }

        if (update_pbuf) {
            gtk_player_update_pbuf (gtk_player);
        }

        free (art_url);

        album = NULL;
    }
}

static void
on_playback_status (PlayerctlPlayer *player,
                    PlayerctlPlaybackStatus playback_status, GtkPlayer *self) {
    UNUSED (player);

    switch (playback_status) {
    case PLAYERCTL_PLAYBACK_STATUS_PAUSED:
        gtk_button_set_child (self->play_button, gtk_image_new_from_icon_name (
                                                     "media-playback-start-symbolic"));
        break;
    case PLAYERCTL_PLAYBACK_STATUS_PLAYING:
        gtk_button_set_child (self->play_button, gtk_image_new_from_icon_name (
                                                     "media-playback-pause-symbolic"));
        break;
    case PLAYERCTL_PLAYBACK_STATUS_STOPPED:
        gtk_button_set_child (self->play_button, gtk_image_new_from_icon_name (
                                                     "media-playback-start-symbolic"));
        break;
    }
}

GtkPlayer *
generate_player (PlayerctlPlayerName *name, MediaControl *self) {
    PlayerctlPlayer *player = playerctl_player_new_from_name (name, NULL);

    playerctl_player_manager_manage_player (self->manager, player);

    GtkBox *box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 2));

    gtk_widget_add_css_class (GTK_WIDGET (box), "control_center_card");
    gtk_widget_set_hexpand (GTK_WIDGET (box), FALSE);
    gtk_widget_set_halign (GTK_WIDGET (box), GTK_ALIGN_FILL);

    // DND icons
    GtkButton *previous_button
        = GTK_BUTTON (gtk_button_new_from_icon_name ("media-skip-backward-symbolic"));
    GtkButton *play_button
        = GTK_BUTTON (gtk_button_new_from_icon_name ("media-playback-start-symbolic"));
    GtkButton *next_button
        = GTK_BUTTON (gtk_button_new_from_icon_name ("media-skip-forward-symbolic"));

    gtk_widget_add_css_class (GTK_WIDGET (previous_button), "media_player_button");
    gtk_widget_add_css_class (GTK_WIDGET (play_button), "media_player_button");
    gtk_widget_add_css_class (GTK_WIDGET (next_button), "media_player_button");

    g_signal_connect (previous_button, "clicked", G_CALLBACK (previous),
                      player);
    g_signal_connect (play_button, "clicked", G_CALLBACK (play_pause), player);
    g_signal_connect (next_button, "clicked", G_CALLBACK (next), player);

    GtkBox *controls_box
        = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2));

    gtk_widget_set_halign (GTK_WIDGET (controls_box), GTK_ALIGN_CENTER);

    gtk_box_append (controls_box, GTK_WIDGET (previous_button));
    gtk_box_append (controls_box, GTK_WIDGET (play_button));
    gtk_box_append (controls_box, GTK_WIDGET (next_button));

    GtkLabel *label = GTK_LABEL (gtk_label_new ("Loading..."));
    gtk_label_set_max_width_chars (label, 0);

    gtk_widget_set_hexpand (GTK_WIDGET (label), TRUE);
    gtk_label_set_wrap (label, TRUE);
    gtk_label_set_wrap_mode (label, PANGO_WRAP_WORD_CHAR);

    GValue playback_status_val = G_VALUE_INIT;
    g_object_get_property (G_OBJECT (player), "playback-status",
                           &playback_status_val);

    PlayerctlPlaybackStatus playback_status
        = g_value_get_enum (&playback_status_val);

    switch (playback_status) {
    case PLAYERCTL_PLAYBACK_STATUS_PAUSED:
        gtk_button_set_child (play_button, gtk_image_new_from_icon_name (
                                               "media-playback-start-symbolic"));
        break;
    case PLAYERCTL_PLAYBACK_STATUS_PLAYING:
        gtk_button_set_child (play_button, gtk_image_new_from_icon_name (
                                               "media-playback-pause-symbolic"));
        break;
    case PLAYERCTL_PLAYBACK_STATUS_STOPPED:
        gtk_button_set_child (play_button, gtk_image_new_from_icon_name (
                                               "media-playback-start-symbolic"));
        break;
    }

    // Dialog icon
    GtkImage *icon = GTK_IMAGE (gtk_image_new_from_icon_name (name->name));

    gtk_widget_add_css_class (GTK_WIDGET (icon), "media_player_icon");

    gtk_widget_set_halign (GTK_WIDGET (icon), GTK_ALIGN_START);

    GtkBox *info_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8));

    gtk_box_append (info_box, GTK_WIDGET (icon));
    gtk_box_append (info_box, GTK_WIDGET (label));

    gtk_box_append (box, GTK_WIDGET (info_box));
    gtk_box_append (box, GTK_WIDGET (controls_box));

    GtkPlayer *return_val = malloc (sizeof (GtkPlayer));

    return_val->player = player;
    return_val->widget = GTK_WIDGET (box);
    return_val->self = self;
    return_val->label = label;
    return_val->bg_path = NULL;
    return_val->width = 0;
    return_val->play_button = play_button;
    return_val->pbuf_blurred = NULL;

    g_signal_connect (player, "metadata", G_CALLBACK (on_metadata),
                      return_val);
    g_signal_connect (player, "playback_status",
                      G_CALLBACK (on_playback_status), return_val);

    on_metadata (player, NULL, return_val);

    return return_val;
}

void
on_name_appeared (PlayerctlPlayerManager *manager, PlayerctlPlayerName *name,
                  MediaControl *self) {
    UNUSED (manager);

    GtkPlayer *player_gtk = generate_player (name, self);

    gtk_box_append (self->box, player_gtk->widget);

    self->gtk_players = g_list_append (self->gtk_players, player_gtk);
}

void
player_vanished_foreach (GtkPlayer *player, PlayerctlPlayer *remove_player) {
    if (player->player == remove_player) {
        gtk_box_remove (GTK_BOX (gtk_widget_get_parent (player->widget)),
                        player->widget);

        g_object_unref (remove_player);

        player->self->gtk_players
            = g_list_remove (player->self->gtk_players, player);

        free (player);
    }
}

void
on_player_vanished (PlayerctlPlayerManager *manager, PlayerctlPlayer *player,
                    MediaControl *self) {
    UNUSED (manager);

    g_list_foreach (self->gtk_players, (GFunc)player_vanished_foreach, player);
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

    g_signal_connect (self->manager, "name-appeared",
                      G_CALLBACK (on_name_appeared), self);
    g_signal_connect (self->manager, "player-vanished",
                      G_CALLBACK (on_player_vanished), self);

    self->box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

    GList *names = playerctl_list_players (NULL);

    g_list_foreach (names, (GFunc)add_name, self);

    g_list_free_full (names, (GDestroyNotify)playerctl_player_name_free);

    return self;
}