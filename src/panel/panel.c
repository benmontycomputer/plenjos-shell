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
#include "tray/panel-tray-menu.h"
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

static void
draw (GtkDrawingArea *drawing_area, cairo_t *cr, int wi, int hi, Panel *self) {
    UNUSED (drawing_area);
    UNUSED (self);

    double width = (double)wi;
    double height = (double)hi - 36.0;

    double x = 0.0;
    double y = 36.0;

    double inset_x = 18.0;

    double radius = 6.0;
    double degrees = M_PI / 180.0;

    double angle = atan2 (inset_x, height);

    cairo_save (cr);

    cairo_move_to (cr, x + inset_x + radius, y);

    cairo_line_to (cr, x + width - inset_x - radius, y);
    cairo_arc (cr, x + width - inset_x - radius, y + radius, radius,
               270 * degrees, (360 * degrees) - angle);

    cairo_line_to (cr, x + width, y + height - 7.0);
    cairo_line_to (cr, x + width, y + height - 4.0);
    cairo_line_to (cr, x, y + height - 4.0);

    cairo_move_to (cr, x + inset_x + radius, y);
    cairo_arc_negative (cr, x + inset_x + radius, y + radius, radius,
                        (270 * degrees), 180 * degrees + angle);

    cairo_line_to (cr, x, y + height - 7.0);
    cairo_line_to (cr, x, y + height - 4.0);

    /* cairo_move_to (cr, x + inset_x, y);
    cairo_line_to (cr, x + width - inset_x, y);
    cairo_line_to (cr, x + width, y + height - 2.0);
    cairo_line_to (cr, x, y + height - 2.0);
    cairo_line_to (cr, x + inset_x, y); */

    cairo_set_source_rgba (cr, 0.4, 0.4, 0.4, 0.5);

    cairo_stroke_preserve (cr);

    cairo_clip (cr);

    // cairo_set_source_rgba (cr, 0.7, 0.7, 0.7, 1.0);

    cairo_pattern_t *pattern = cairo_pattern_create_linear (0, y, 0, y + height - 4.0);

    cairo_pattern_add_color_stop_rgba (pattern, 0.0, 0.6, 0.6, 0.6, 1.0);
    cairo_pattern_add_color_stop_rgba (pattern, 1.0, 0.7, 0.7, 0.7, 1.0);

    cairo_set_source (cr, pattern);

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

    cairo_paint (cr);

    cairo_pattern_destroy (pattern);

    cairo_restore (cr);

    cairo_save (cr);

    // cairo_move_to (cr, x, y + height - 4.0);
    // cairo_line_to (cr, x + width, y + height - 4.0);
    cairo_new_path (cr);
    cairo_arc_negative (cr, x + 3.0, y + height - 7.0, 3.0, 180 * degrees,
                        90 * degrees);
    cairo_arc_negative (cr, x + width - 3.0, y + height - 7.0, 3.0, 90 * degrees,
                        0 * degrees);

    cairo_arc (cr, x + width - 3.0, y + height - 3.0, 3.0, 0, 90 * degrees);
    cairo_arc (cr, x + 3.0, y + height - 3.0, 3.0, 90 * degrees,
               180 * degrees);
    cairo_close_path (cr);

    cairo_set_source_rgba (cr, 0.4, 0.4, 0.4, 0.5);

    cairo_stroke_preserve (cr);

    cairo_clip (cr);

    /*cairo_move_to (cr, x, y + height - 1.0);
    cairo_line_to (cr, x + width, y + height - 1.0);

    cairo_set_source_rgba (cr, 0.8, 0.8, 0.8, 1.0);

    cairo_set_line_width (cr, 2.0);
    cairo_stroke (cr);*/
    cairo_set_source_rgba (cr, 0.8, 0.8, 0.8, 1.0);

    cairo_paint (cr);

    cairo_restore (cr);
}

static void
activate (GtkApplication *app, void *_data) {
    (void)_data;

    GtkCssProvider *cssProvider = gtk_css_provider_new ();
    gtk_css_provider_load_from_resource (cssProvider,
                                         "/com/plenjos/Panel/theme.css");
    gtk_style_context_add_provider_for_display (
        gdk_display_get_default (), GTK_STYLE_PROVIDER (cssProvider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Create a normal GTK window however you like
    GtkWindow *gtk_window = GTK_WINDOW (gtk_application_window_new (app));
    GtkWindow *gtk_window_2 = GTK_WINDOW (gtk_application_window_new (app));

    Panel *self = malloc (sizeof (Panel));
    self->blurred = NULL;
    self->gtk_window = gtk_window;
    self->supports_alpha = FALSE;
    self->taskbar = NULL;

    // Before the window is first realized, set it up to be a layer surface
    gtk_layer_init_for_window (gtk_window);

    // Order below normal windows
    gtk_layer_set_layer (gtk_window, GTK_LAYER_SHELL_LAYER_TOP);

    // Push other windows out of the way
    gtk_layer_auto_exclusive_zone_enable (gtk_window);

    // We don't need to get keyboard input
    // gtk_layer_set_keyboard_mode (gtk_window,
    // GTK_LAYER_SHELL_KEYBOARD_MODE_NONE); // NONE is default

    // The margins are the gaps around the window's edges
    // Margins and anchors can be set like this...
    /*static const gint margins[] = { 2, 2, 2, 2 };
    for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++) {
        gtk_layer_set_margin (gtk_window, i, margins[i]);
    }*/

    // ... or like this
    // Anchors are if the window is pinned to each edge of the output
    static const gboolean anchors[] = { TRUE, TRUE, FALSE, TRUE };
    for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++) {
        gtk_layer_set_anchor (gtk_window, i, anchors[i]);
    }

    // Before the window is first realized, set it up to be a layer surface
    gtk_layer_init_for_window (gtk_window_2);

    // Order below normal windows
    gtk_layer_set_layer (gtk_window_2, GTK_LAYER_SHELL_LAYER_TOP);

    // Push other windows out of the way
    gtk_layer_auto_exclusive_zone_enable (gtk_window_2);

    // We don't need to get keyboard input
    // gtk_layer_set_keyboard_mode (gtk_window,
    // GTK_LAYER_SHELL_KEYBOARD_MODE_NONE); // NONE is default

    // The margins are the gaps around the window's edges
    // Margins and anchors can be set like this...
    /*static const gint margins[] = { 2, 2, 2, 2 };
    for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++) {
        gtk_layer_set_margin (gtk_window, i, margins[i]);
    }*/

    // ... or like this
    // Anchors are if the window is pinned to each edge of the output
    static const gboolean anchors_2[] = { TRUE, TRUE, TRUE, FALSE };
    for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++) {
        gtk_layer_set_anchor (gtk_window_2, i, anchors_2[i]);
    }

    gtk_widget_set_size_request (GTK_WIDGET (gtk_window_2), 480, 32);

    // gtk_widget_set_size_request (GTK_WIDGET (gtk_window), 480, 56);
    gtk_widget_set_name (GTK_WIDGET (gtk_window), "panel_window");

    GtkBox *panel_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_widget_set_name (GTK_WIDGET (panel_box), "panel_box");
    gtk_widget_set_halign (GTK_WIDGET (panel_box), GTK_ALIGN_CENTER);

    GtkBox *menu_bar_box
        = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_widget_set_name (GTK_WIDGET (menu_bar_box), "menu_bar_box");

    PanelApplicationsMenu *apps_menu
        = g_object_new (PANEL_TYPE_APPLICATIONS_MENU, NULL);

    gtk_box_append (
        panel_box,
        GTK_WIDGET (panel_applications_menu_get_launcher_button (apps_menu)));

    hide_applications_menu (apps_menu);

    PanelTaskbar *panel_taskbar = panel_taskbar_init ();

    self->taskbar = panel_taskbar;

    gtk_box_append (panel_box, GTK_WIDGET (panel_taskbar->taskbar_box));

    GTask *task = g_task_new (gtk_window, NULL, NULL, NULL);
    g_task_set_task_data (task, panel_taskbar, NULL);
    g_task_run_in_thread (task, (GTaskThreadFunc)panel_taskbar_run_wrap);

    PanelTray *panel_tray = panel_tray_new ((gpointer)self);

    gtk_box_append (menu_bar_box, GTK_WIDGET (panel_tray->tray_box));

    gtk_widget_set_hexpand (GTK_WIDGET (panel_tray->tray_box), TRUE);

    GtkBox *box2 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
    gtk_widget_set_halign (GTK_WIDGET (box2), GTK_ALIGN_CENTER);

    GtkDrawingArea *da = GTK_DRAWING_AREA (gtk_drawing_area_new ());

    gtk_widget_set_size_request (GTK_WIDGET (da), 0, 64);

    gtk_widget_set_halign (GTK_WIDGET (da), GTK_ALIGN_FILL);
    gtk_widget_set_hexpand (GTK_WIDGET (da), TRUE);

    gtk_widget_set_margin_bottom (GTK_WIDGET (da), -72);

    gtk_widget_set_hexpand (GTK_WIDGET (da), TRUE);
    gtk_widget_set_vexpand (GTK_WIDGET (da), TRUE);

    gtk_drawing_area_set_draw_func (da, (GtkDrawingAreaDrawFunc)draw, self,
                                    NULL);

    gtk_box_append (box2, GTK_WIDGET (da));
    gtk_box_append (box2, GTK_WIDGET (panel_box));

    gtk_window_set_child (gtk_window, GTK_WIDGET (box2));
    gtk_window_set_child (gtk_window_2, GTK_WIDGET (menu_bar_box));

    gtk_widget_show (GTK_WIDGET (gtk_window));
    gtk_widget_show (GTK_WIDGET (gtk_window_2));

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

    /*int scale_factor = gdk_monitor_get_scale_factor (
        gdk_display_get_monitor_at_window (gdk_display_get_default (), win));

    cairo_surface_t *surfaceold
        = gdk_cairo_surface_create_from_pixbuf (pbuf, 1, win);

    cairo_surface_t *surface = cairo_image_surface_create (
        CAIRO_FORMAT_RGB24, cairo_image_surface_get_width (surfaceold),
        cairo_image_surface_get_height (surfaceold));

    cairo_surface_set_device_scale (surface, scale_factor, scale_factor);

    stack_blur (surfaceold, surface, 240, 240);

    GdkPixbuf *unscaled
        = gdk_pixbuf_get_from_surface (surface, 0, 0, 2560, 1440);

    GdkPixbuf *scaled
        = gdk_pixbuf_scale_simple (unscaled, 2560, 1440, GDK_INTERP_BILINEAR);

    g_object_unref (unscaled);
    g_object_unref (pbuf);
    cairo_surface_destroy (surfaceold);
    cairo_surface_destroy (surface);

    self->blurred = scaled;*/

    self->blurred = pbuf;

    panel_applications_menu_set_bg (apps_menu, self->blurred);
}

int
main (int argc, char **argv) {
    GtkApplication *app = gtk_application_new ("com.plenjos.plenjos-panel", 0);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    int status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);
    return status;
}