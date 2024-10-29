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

#include "taskbar/panel-taskbar.h"
#include "tray/panel-tray.h"

#include "applications-menu/panel-applications-menu.h"

void
panel_taskbar_run_wrap (GTask *task, GObject *source_object,
                        gpointer task_data, GCancellable *cancellable) {
    UNUSED (task);
    UNUSED (source_object);
    UNUSED (cancellable);

    PanelTaskbar *taskbar = (PanelTaskbar *)task_data;

    panel_taskbar_run (taskbar);
}

void
hypr_backend_run_wrap (GTask *task, GObject *source_object,
                           gpointer task_data, GCancellable *cancellable) {
    UNUSED (task);
    UNUSED (source_object);
    UNUSED (cancellable);

    HyprBackend *backend = (HyprBackend *)task_data;

    hypr_backend_run (backend);
}

static void
monitors_changed (GListModel *monitors, guint position, guint removed,
                  guint added, Panel *self) {
    panel_tray_update_monitors ((PanelTray *)self->tray);

    panel_taskbar_update_monitors (self->taskbar);
}

static void
activate (GtkApplication *app, void *_data) {
    (void)_data;

    notifyd_init ();

    GtkCssProvider *cssProvider = gtk_css_provider_new ();
    gtk_css_provider_load_from_resource (cssProvider,
                                         "/com/plenjos/Panel/theme.css");
    gtk_style_context_add_provider_for_display (
        gdk_display_get_default (), GTK_STYLE_PROVIDER (cssProvider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    Panel *self = malloc (sizeof (Panel));
    self->app = app;
    self->supports_alpha = FALSE;
    self->taskbar = NULL;
    self->panel_settings = g_settings_new ("com.plenjos.shell.panel");

    PanelTaskbar *panel_taskbar = panel_taskbar_init (self);

    self->taskbar = panel_taskbar;

    GTask *taskbar_task = g_task_new (app, NULL, NULL, NULL);
    g_task_set_task_data (taskbar_task, panel_taskbar, NULL);
    g_task_run_in_thread (taskbar_task,
                          (GTaskThreadFunc)panel_taskbar_run_wrap);

    self->tray = (gpointer)panel_tray_new ((gpointer)self);

    // GdkWindow *win = gtk_widget_get_window (GTK_WIDGET (gtk_window));

    GSettings *settings = g_settings_new ("org.gnome.desktop.interface");
    GSettings *bg_settings = g_settings_new ("org.gnome.desktop.background");

    char *bg = NULL;
    char *dark = g_settings_get_string (settings, "color-scheme");
    if (dark) {
        if (!strcmp (dark, "prefer-dark")) {
            bg = g_settings_get_string (bg_settings, "picture-uri-dark");
            self->dark_mode = TRUE;
        } else {
            bg = g_settings_get_string (bg_settings, "picture-uri");
            self->dark_mode = FALSE;
        }

        free (dark);
    }

    GdkPixbuf *pbuf = NULL;

    // "/usr/share/backgrounds/gnome/drool-d.svg"
    if (bg != NULL) {
        pbuf = gdk_pixbuf_new_from_file_at_size (bg + 7, 2560, -1, NULL);

        free (bg);
    } else {
        pbuf = gdk_pixbuf_new_from_file_at_size (
            "/usr/share/backgrounds/gnome/drool-d.svg", 2560, -1, NULL);
    }

    GdkDisplay *display = gdk_display_get_default ();

    // This will update automatically
    self->monitors = gdk_display_get_monitors (display);

    panel_tray_update_monitors ((PanelTray *)self->tray);
    panel_taskbar_update_monitors (self->taskbar);

    g_signal_connect (self->monitors, "items-changed",
                      G_CALLBACK (monitors_changed), self);

    HyprBackend *hypr = hypr_backend_init (self);

    GTask *hypr_task = g_task_new (app, NULL, NULL, NULL);
    g_task_set_task_data (hypr_task, hypr, NULL);
    g_task_run_in_thread (hypr_task,
                          (GTaskThreadFunc)hypr_backend_run_wrap);
}

int
main (int argc, char **argv) {
    GtkApplication *app = gtk_application_new ("com.plenjos.plenjos-panel", 0);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    int status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);
    return status;
}