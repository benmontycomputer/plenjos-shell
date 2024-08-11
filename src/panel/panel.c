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

#include "panel.h"

typedef struct {
    PanelTaskbar *taskbar;
} Panel;

void
panel_taskbar_run_wrap (GTask *task, GObject *source_object,
                        gpointer task_data, GCancellable *cancellable) {
    UNUSED (task);
    UNUSED (source_object);
    UNUSED (cancellable);

    PanelTaskbar *taskbar = (PanelTaskbar *)task_data;

    panel_taskbar_run (taskbar);
}
static void
activate (GtkApplication *app, void *_data) {
    (void)_data;

    GtkCssProvider *cssProvider = gtk_css_provider_new ();
    gtk_css_provider_load_from_resource (cssProvider,
                                         "/com/plenjos/Panel/theme.css");
    gtk_style_context_add_provider_for_screen (
        gdk_screen_get_default (), GTK_STYLE_PROVIDER (cssProvider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Create a normal GTK window however you like
    GtkWindow *gtk_window = GTK_WINDOW (gtk_application_window_new (app));

    // Before the window is first realized, set it up to be a layer surface
    gtk_layer_init_for_window (gtk_window);

    // Order below normal windows
    gtk_layer_set_layer (gtk_window, GTK_LAYER_SHELL_LAYER_BOTTOM);

    // Push other windows out of the way
    gtk_layer_auto_exclusive_zone_enable (gtk_window);

    // We don't need to get keyboard input
    // gtk_layer_set_keyboard_mode (gtk_window,
    // GTK_LAYER_SHELL_KEYBOARD_MODE_NONE); // NONE is default

    // The margins are the gaps around the window's edges
    // Margins and anchors can be set like this...
    gtk_layer_set_margin (gtk_window, GTK_LAYER_SHELL_EDGE_TOP, 2);

    // ... or like this
    // Anchors are if the window is pinned to each edge of the output
    static const gboolean anchors[] = { TRUE, TRUE, FALSE, TRUE };
    for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++) {
        gtk_layer_set_anchor (gtk_window, i, anchors[i]);
    }

    gtk_widget_set_size_request (GTK_WIDGET (gtk_window), 480, 56);

    GtkBox *panel_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_widget_set_name (GTK_WIDGET (panel_box), "panel_box");

    PanelApplicationsMenu *apps_menu = g_object_new (PANEL_TYPE_APPLICATIONS_MENU, NULL);

    gtk_box_pack_start (panel_box, GTK_WIDGET (panel_applications_menu_get_launcher_button (apps_menu)), FALSE,
                      FALSE, 0);

    hide_applications_menu (apps_menu);

    PanelTaskbar *panel_taskbar = panel_taskbar_init ();
    gtk_box_pack_start (panel_box,
                       GTK_WIDGET (panel_taskbar->taskbar_box), FALSE, FALSE, 0);

    GTask *task = g_task_new (gtk_window, NULL, NULL, NULL);
    g_task_set_task_data (task, panel_taskbar, NULL);
    g_task_run_in_thread (task, (GTaskThreadFunc)panel_taskbar_run_wrap);

    PanelClock *panel_clock = panel_clock_new ();

    g_timeout_add (500, (GSourceFunc)panel_clock_update, panel_clock);

    gtk_box_pack_end (panel_box, GTK_WIDGET (panel_clock->label), FALSE,
                      FALSE, 0);

    gtk_container_add (GTK_CONTAINER (gtk_window), GTK_WIDGET (panel_box));

    gtk_widget_show_all (GTK_WIDGET (panel_box));
    gtk_widget_show_all (GTK_WIDGET (gtk_window));
}

int
main (int argc, char **argv) {
    GtkApplication *app = gtk_application_new ("com.plenjos.plenjos-panel",
                                               G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    int status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);
    return status;
}
