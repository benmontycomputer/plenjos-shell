/* This entire file is licensed under MIT
 *
 * Copyright 2020 Sophie Winter
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "desktop.h"

static void update_monitors (Desktop *self) {
    if (self->windows) {
        for (size_t i = 0; self->windows[i]; i++) {
            GtkWindow *win = self->windows[i];

            gtk_window_close (win);
        }
    }

    size_t n_monitors = g_list_model_get_n_items (self->monitors);

    self->windows = malloc ((n_monitors + 1) * sizeof (GtkWindow *));

    for (size_t i = 0; i < n_monitors; i++) {
        // Create a normal GTK window however you like
        GtkWindow *gtk_window = GTK_WINDOW (gtk_application_window_new (self->app));

        // Before the window is first realized, set it up to be a layer surface
        gtk_layer_init_for_window (gtk_window);

        // Order below normal windows
        gtk_layer_set_layer (gtk_window, GTK_LAYER_SHELL_LAYER_BACKGROUND);

        // Push other windows out of the way
        gtk_layer_set_exclusive_zone (gtk_window, -1);

        // We don't need to get keyboard input
        // gtk_layer_set_keyboard_mode (gtk_window,
        // GTK_LAYER_SHELL_KEYBOARD_MODE_NONE); // NONE is default

        // The margins are the gaps around the window's edges
        // Margins and anchors can be set like this...

        // ... or like this
        // Anchors are if the window is pinned to each edge of the output
        static const gboolean anchors[] = { TRUE, TRUE, TRUE, TRUE };
        for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++) {
            gtk_layer_set_anchor (gtk_window, i, anchors[i]);
        }

        gtk_widget_set_size_request (GTK_WIDGET (gtk_window), 2560, -1);
        gtk_widget_set_name (GTK_WIDGET (gtk_window), "desktop_window");

        GSettings *settings = g_settings_new ("org.gnome.desktop.interface");
        GSettings *bg_settings = g_settings_new ("org.gnome.desktop.background");

        char *bg = NULL;
        if (!strcmp (g_settings_get_string (settings, "color-scheme"),
                    "prefer-dark")) {
            bg = g_settings_get_string (bg_settings, "picture-uri-dark");
        } else {
            bg = g_settings_get_string (bg_settings, "picture-uri");
        }

        GtkWidget *img = gtk_image_new_from_file (bg + 7);
        gtk_widget_set_size_request (GTK_WIDGET (img), 2560, 2560);

        gtk_window_set_child (gtk_window, img);

        gtk_layer_set_monitor (gtk_window, g_list_model_get_item (self->monitors, i));
        gtk_window_present (gtk_window);

        printf("%d\n", i);
        fflush(stdout);

        self->windows[i] = gtk_window;
    }

    self->windows[n_monitors] = NULL;
}

static void
monitors_changed (GListModel *monitors, guint position, guint removed,
                  guint added, Desktop *self) {
    update_monitors (self);
}

static void
activate (GtkApplication *app, void *_data) {
    (void)_data;

    // GtkCssProvider *cssProvider = gtk_css_provider_new ();
    // gtk_css_provider_load_from_resource (cssProvider,
    //                                      "/com/plenjos/Desktop/theme.css");
    // gtk_style_context_add_provider_for_screen (
    //     gdk_screen_get_default (), GTK_STYLE_PROVIDER (cssProvider),
    //     GTK_STYLE_PROVIDER_PRIORITY_USER);

    Desktop *self = malloc (sizeof (Desktop *));

    self->windows = NULL;
    self->app = app;

    GdkDisplay *display = gdk_display_get_default ();

    // This will update automatically
    self->monitors = gdk_display_get_monitors (display);

    update_monitors (self);

    g_signal_connect (self->monitors, "items-changed", G_CALLBACK (monitors_changed), self);
}

int
main (int argc, char **argv) {
    GtkApplication *app
        = gtk_application_new ("com.plenjos.plenjos-desktop", 0);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    int status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);
    return status;
}
