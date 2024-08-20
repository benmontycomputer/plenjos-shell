#include "notifyd.h"

#define NOTIFICATIONS_SPEC_VERSION "1.2"

typedef struct _NotifyDaemon {
    PlenjosNotifyFdoGBusSkeleton *skeleton;
} NotifyDaemon;

const char *capabilities[] = {
    "action-icons",    "actions",     "body",
    "body-hyperlinks", "body-images", "body-markup",
    "icon-multi",      "persistence", "sound",
};

static const char **
on_get_capabilities () {
    return capabilities;
}

gboolean close_notify (GtkWindow *win) {
    gtk_window_close (win);

    return FALSE;
}

static gboolean
on_notify (PlenjosNotifyFdoGBusSkeleton *skeleton,
           GDBusMethodInvocation *invocation, const gchar *app_name,
           guint replaces_id, gchar *app_icon, gchar *summary, gchar *body,
           gchar **actions_data, GVariant *hints, gint expire_timeout,
           gpointer user_data) {
    printf ("App name: %s, replaces_id: %u, app_icon: %s, summary: %s, body: "
            "%s, actions_data: %s, expire_timeout: %d\n",
            app_name, replaces_id, app_icon, summary, body, NULL, expire_timeout);
    fflush (stdout);

    GtkWindow *win = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));

    // Before the window is first realized, set it up to be a layer surface
    gtk_layer_init_for_window (win);

    // Order below normal windows
    gtk_layer_set_layer (win, GTK_LAYER_SHELL_LAYER_TOP);

    // We don't need to get keyboard input
    // gtk_layer_set_keyboard_mode (gtk_window,
    // GTK_LAYER_SHELL_KEYBOARD_MODE_NONE); // NONE is default

    // The margins are the gaps around the window's edges
    // Margins and anchors can be set like this...
    gtk_layer_set_margin (win, GTK_LAYER_SHELL_EDGE_TOP, 24);

    // ... or like this
    // Anchors are if the window is pinned to each edge of the output
    static const gboolean anchors[] = { FALSE, FALSE, TRUE, FALSE };
    for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++) {
        gtk_layer_set_anchor (win, i, anchors[i]);
    }

    gtk_widget_set_size_request (GTK_WIDGET (win), 480, 120);

    gtk_window_set_decorated (win, FALSE);
    gtk_window_set_deletable (win, FALSE);
    gtk_window_set_resizable (win, FALSE);

    gtk_container_add (GTK_CONTAINER (win), gtk_label_new (body));

    gtk_widget_show_all (GTK_WIDGET (win));

    gtk_window_present (win);
    
    g_timeout_add (expire_timeout == -1 ? 5000 : expire_timeout, G_SOURCE_FUNC (close_notify), win);

    plenjos_notify_fdo_gbus_complete_notify (skeleton, invocation, 0);

    return TRUE;
}

static gboolean
on_get_server_information (PlenjosNotifyFdoGBusSkeleton *skeleton,
                           GDBusMethodInvocation *invocation,
                           NotifyDaemon *self) {
    printf ("get server information\n");
    fflush (stdout);

    plenjos_notify_fdo_gbus_complete_get_server_information (
        skeleton, invocation, "PlenjOS Notify Daemon", "PlenjOS", "0.0.1",
        NOTIFICATIONS_SPEC_VERSION);

    return TRUE;
}

static void
on_name_acquired (GDBusConnection *connection, const char *name,
                  gpointer user_data) {
    UNUSED (user_data);

    PlenjosNotifyFdoGBusSkeleton *skeleton
        = plenjos_notify_fdo_gbus_skeleton_new ();

    GError *error = NULL;

    gboolean exported;

    if (strcmp (name, "org.freedesktop.Notifications") == 0) {
        NotifyDaemon *self = malloc (sizeof (NotifyDaemon));

        self->skeleton = skeleton;

        exported = g_dbus_interface_skeleton_export (
            G_DBUS_INTERFACE_SKELETON (skeleton), connection,
            "/org/freedesktop/Notifications", &error);

        if (exported) {
            g_signal_connect (self->skeleton, "handle-notify",
                              G_CALLBACK (on_notify), self);

            g_signal_connect (self->skeleton, "handle-get-capabilities",
                              G_CALLBACK (on_get_capabilities), self);

            g_signal_connect (self->skeleton, "handle-get-server-information",
                              G_CALLBACK (on_get_server_information), self);

            // g_signal_connect (self, "handle-close-notification", G_CALLBACK
            // (on_close_notification), self);
        } else {
            printf ("fail\n");
            fflush (stdout);
        }
    }
}

int
main (int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        printf("%d: %s\n", argv[i]);
    }

    fflush (stdout);

    gtk_init (NULL, NULL);

    GMainLoop *loop;

    loop = g_main_loop_new (NULL, FALSE);
    g_bus_own_name (G_BUS_TYPE_SESSION, "org.freedesktop.Notifications",
                    G_BUS_NAME_OWNER_FLAGS_NONE, NULL, on_name_acquired, NULL,
                    NULL, NULL);

    g_main_loop_run (loop);
}