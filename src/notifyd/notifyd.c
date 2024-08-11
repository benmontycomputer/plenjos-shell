#include "notifyd.h"

// Some code adapted from https://github.com/makercrew/dbus-sample/

/*int
main (int argc, char **argv) {
    DBusError dbus_error;
    DBusConnection *dbus_connection = NULL;
    DBusMessage *dbus_message = NULL;
    DBusMessage *dbus_reply = NULL;

    const char *dbus_result = NULL;

    dbus_error_init (&dbus_error);

    if (NULL == (dbus_connection = dbus_bus_get (DBUS_BUS_SYSTEM,
&dbus_error))) { fprintf (stderr, "Error initializing DBUS: %s, message: %s\n",
dbus_error.name, dbus_error.message); fflush (stderr); } else if (NULL ==
(dbus_message = dbus_message_new_method_call ("org.freedesktop.DBus", "/",
"org.freedesktop.DBus.Introspectable", "Introspect"))) { dbus_connection_unref
(dbus_connection);

        fprintf (stderr, "Error: dbus_message_new_method_call - Unable to
allocate memory for the message!\n"); fflush (stderr); } else if (NULL ==
(dbus_reply = dbus_connection_send_with_reply_and_block (dbus_connection,
dbus_message, DBUS_TIMEOUT_USE_DEFAULT, &dbus_error))) { dbus_message_unref
(dbus_message); dbus_connection_unref (dbus_connection); fprintf (stderr,
"Error in dbus_connection_send_with_reply_and_block: %s, message: %s\n",
dbus_error.name, dbus_error.message); fflush (stderr); } else if
(!dbus_message_get_args (dbus_reply, &dbus_error, DBUS_TYPE_STRING,
&dbus_result, DBUS_TYPE_INVALID)) { dbus_message_unref (dbus_message);
        dbus_message_unref (dbus_reply);
        dbus_connection_unref (dbus_connection);
        fprintf (stderr, "Error in dbus_message_get_args: %s, message: %s\n",
dbus_error.name, dbus_error.message); fflush (stderr); } else { printf
("Conected to D-Bus as \"%s\"\n", dbus_bus_get_unique_name (dbus_connection));
        printf ("Introspection result: %s\n", dbus_result);
        fflush(stdout);

        dbus_message_unref (dbus_message);
        dbus_message_unref (dbus_reply);
        dbus_connection_unref (dbus_connection);
    }
}*/

static void
on_name_acquired (GDBusConnection *connection, const char *name,
                  gpointer user_data) {
    
}

int
main (int argc, char **argv) {
    GMainLoop *loop;

    loop = g_main_loop_new (NULL, FALSE);
    g_bus_own_name (G_BUS_TYPE_SESSION, "org.freedesktop.Notifications",
                    G_BUS_NAME_OWNER_FLAGS_NONE, NULL, on_name_acquired, NULL,
                    NULL, NULL);

    g_main_loop_run (loop);
}