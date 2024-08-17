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

    GdkPixbuf *blurred;

    GtkWindow *gtk_window;

    bool supports_alpha;
    bool dark_mode;
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

gboolean
expose_draw (GtkWidget *widget, cairo_t *cr, Panel *self) {
    cairo_save (cr);

    // Adapted from
    // https://stackoverflow.com/questions/4183546/how-can-i-draw-image-with-rounded-corners-in-cairo-gtk
    double width, height, radius;
    int wi, hi;
    gtk_window_get_size (self->gtk_window, &wi, &hi);

    width = (double)wi;
    height = (double)hi;
    radius = 12;

    double x = 0.0;
    double y = 0.0;

    x += 6.0;
    y += 6.0;
    width -= 12.0;
    height -= 12.0;

    double degrees = M_PI / 180.0;

    // cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);

    /*cairo_set_line_width (cr, 2.0);
    cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees,
               0 * degrees);
    cairo_arc (cr, x + width - radius, y + height - radius, radius,
               0 * degrees, 90 * degrees);
    cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees,
               180 * degrees);
    cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees,
               270 * degrees);
    cairo_line_to (cr, x + width - radius, y);

    cairo_stroke (cr);*/

    cairo_pattern_t *pattern = cairo_pattern_create_radial (
        x + radius, y + radius, radius, x + radius, y + radius, radius + 6.0);

    cairo_pattern_add_color_stop_rgba (pattern, 0, 0, 0, 0, 0.6);
    cairo_pattern_add_color_stop_rgba (pattern, 0.6, 0, 0, 0, 0.2);
    cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 0.0);

    cairo_save (cr);

    cairo_rectangle (cr, x - 6.0, y - 6.0, radius + 6.0, radius + 6.0);

    cairo_clip (cr);
    
    cairo_set_source (cr, pattern);
    cairo_mask (cr, pattern);

    cairo_pattern_destroy (pattern);

    cairo_restore (cr);

    pattern = cairo_pattern_create_radial (x + width - radius, y + radius,
                                           radius, x + width - radius,
                                           y + radius, radius + 6.0);

    cairo_pattern_add_color_stop_rgba (pattern, 0, 0, 0, 0, 0.6);
    cairo_pattern_add_color_stop_rgba (pattern, 0.6, 0, 0, 0, 0.2);
    cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 0.0);

    cairo_save (cr);

    cairo_rectangle (cr, x + width - radius, y - 6.0, radius + 6.0, radius + 6.0);
    
    cairo_clip (cr);

    cairo_set_source (cr, pattern);
    cairo_mask (cr, pattern);

    cairo_pattern_destroy (pattern);

    cairo_restore (cr);

    pattern = cairo_pattern_create_radial (
        x + width - radius, y + height - radius, radius, x + width - radius,
        y + height - radius, radius + 6.0);

    cairo_pattern_add_color_stop_rgba (pattern, 0, 0, 0, 0, 0.6);
    cairo_pattern_add_color_stop_rgba (pattern, 0.6, 0, 0, 0, 0.2);
    cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 0.0);

    cairo_save (cr);

    cairo_rectangle (cr, x + width - radius, y + height - radius, radius + 6.0, radius + 6.0);

    cairo_clip (cr);

    cairo_set_source (cr, pattern);
    cairo_mask (cr, pattern);

    cairo_pattern_destroy (pattern);

    cairo_restore (cr);

    pattern = cairo_pattern_create_radial (x + radius, y + height - radius,
                                           radius, x + radius,
                                           y + height - radius, radius + 6.0);

    cairo_pattern_add_color_stop_rgba (pattern, 0, 0, 0, 0, 0.6);
    cairo_pattern_add_color_stop_rgba (pattern, 0.6, 0, 0, 0, 0.2);
    cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 0.0);

    cairo_save (cr);

    cairo_rectangle (cr, x - 6.0, y + height - radius, radius + 6.0, radius + 6.0);

    cairo_clip (cr);

    cairo_set_source (cr, pattern);
    cairo_mask (cr, pattern);

    cairo_pattern_destroy (pattern);

    cairo_restore (cr);

    // Bottom shadow
    pattern = cairo_pattern_create_linear (x + (width / 2), y + height,
                                           x + (width / 2), y + height + 6.0);

    cairo_pattern_add_color_stop_rgba (pattern, 0, 0, 0, 0, 0.6);
    cairo_pattern_add_color_stop_rgba (pattern, 0.6, 0, 0, 0, 0.2);
    cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 0.0);

    cairo_save (cr);

    cairo_rectangle (cr, x + radius, y + height, width - (2 * radius), 6.0);

    cairo_clip (cr);

    cairo_set_source (cr, pattern);
    cairo_mask (cr, pattern);

    cairo_pattern_destroy (pattern);

    cairo_restore (cr);

    // Top shadow
    pattern = cairo_pattern_create_linear (x + (width / 2), y,
                                           x + (width / 2), y - 6.0);

    cairo_pattern_add_color_stop_rgba (pattern, 0, 0, 0, 0, 0.6);
    cairo_pattern_add_color_stop_rgba (pattern, 0.6, 0, 0, 0, 0.2);
    cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 0.0);

    cairo_save (cr);

    cairo_rectangle (cr, x + radius, y - 6.0, width - (2 * radius), 6.0);

    cairo_clip (cr);

    cairo_set_source (cr, pattern);
    cairo_mask (cr, pattern);

    cairo_pattern_destroy (pattern);

    cairo_restore (cr);

    // Left shadow
    pattern = cairo_pattern_create_linear (x, y + (height / 2),
                                           x - 6.0, y + (height / 2));

    cairo_pattern_add_color_stop_rgba (pattern, 0, 0, 0, 0, 0.6);
    cairo_pattern_add_color_stop_rgba (pattern, 0.6, 0, 0, 0, 0.2);
    cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 0.0);

    cairo_save (cr);

    cairo_rectangle (cr, x - 6.0, y + radius, 6.0, height - (2 * radius));

    cairo_clip (cr);

    cairo_set_source (cr, pattern);
    cairo_mask (cr, pattern);

    cairo_pattern_destroy (pattern);

    cairo_restore (cr);

    // Right shadow
    pattern = cairo_pattern_create_linear (x + width, y + (height / 2),
                                           x + width + 6.0, y + (height / 2));

    cairo_pattern_add_color_stop_rgba (pattern, 0, 0, 0, 0, 0.6);
    cairo_pattern_add_color_stop_rgba (pattern, 0.6, 0, 0, 0, 0.2);
    cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 0.0);

    cairo_save (cr);

    cairo_rectangle (cr, x + width, y + radius, 6.0, height - (2 * radius));

    cairo_clip (cr);

    cairo_set_source (cr, pattern);
    cairo_mask (cr, pattern);

    cairo_pattern_destroy (pattern);

    cairo_restore (cr);

    cairo_arc (cr, x + width - radius, y + radius, radius + 0.5, -90 * degrees,
               0 * degrees);
    cairo_arc (cr, x + width - radius, y + height - radius, radius + 0.5,
               0 * degrees, 90 * degrees);
    cairo_arc (cr, x + radius, y + height - radius, radius + 0.5, 90 * degrees,
               180 * degrees);
    cairo_arc (cr, x + radius, y + radius, radius + 0.5, 180 * degrees,
               270 * degrees);
    cairo_close_path (cr);
    cairo_set_line_width (cr, 1.0);
    cairo_set_source_rgba (cr, 0.8, 0.8, 0.8, 0.4);
    cairo_stroke (cr);

    cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees,
               0 * degrees);
    cairo_arc (cr, x + width - radius, y + height - radius, radius,
               0 * degrees, 90 * degrees);
    cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees,
               180 * degrees);
    cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees,
               270 * degrees);
    cairo_close_path (cr);

    cairo_clip (cr);

    if (GDK_IS_PIXBUF (self->blurred)) {
        gint x_win, y_win;
        gtk_window_get_position (self->gtk_window, &x_win, &y_win);
        gdk_cairo_set_source_pixbuf (cr, self->blurred, -x_win, -1376);
    } else {
        if (self->supports_alpha) {
            cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.0);
        } else {
            cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
        }
    }

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);

    cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees,
               0 * degrees);
    cairo_arc (cr, x + width - radius, y + height - radius, radius,
               0 * degrees, 90 * degrees);
    cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees,
               180 * degrees);
    cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees,
               270 * degrees);
    cairo_close_path (cr);

    cairo_clip (cr);

    if (self->dark_mode) {
        cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 0.7);
    } else {
        cairo_set_source_rgba (cr, 0.8, 0.8, 0.8, 0.5);
    }

    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

    cairo_paint (cr);

    cairo_restore (cr);

    return FALSE;
}

void
screen_changed (GtkWidget *widget, GdkScreen *old_screen, Panel *self) {
    // fix unused parameter warning
    (void)old_screen;
    GdkScreen *screen = gtk_widget_get_screen (widget);
    GdkVisual *visual = gdk_screen_get_rgba_visual (screen);

    if (!visual) {
        printf ("Your screen does not support alpha channels!\n");
        visual = gdk_screen_get_system_visual (screen);
        self->supports_alpha = FALSE;
    } else {
        printf ("Your screen supports alpha channels!\n");
        self->supports_alpha = TRUE;
    }

    fflush (stdout);

    gtk_widget_set_visual (widget, visual);
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

    Panel *self = malloc (sizeof (Panel));
    self->blurred = NULL;
    self->gtk_window = gtk_window;
    self->supports_alpha = FALSE;
    self->taskbar = NULL;

    g_signal_connect (gtk_window, "screen-changed",
                      G_CALLBACK (screen_changed), self);

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
    static const gint margins[] = { 2, 2, 2, 2 };
    for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++) {
        gtk_layer_set_margin (gtk_window, i, margins[i]);
    }

    // ... or like this
    // Anchors are if the window is pinned to each edge of the output
    static const gboolean anchors[] = { TRUE, TRUE, FALSE, TRUE };
    for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++) {
        gtk_layer_set_anchor (gtk_window, i, anchors[i]);
    }

    gtk_widget_set_size_request (GTK_WIDGET (gtk_window), 480, 64);
    gtk_widget_set_name (GTK_WIDGET (gtk_window), "panel_window");

    GtkBox *panel_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_widget_set_name (GTK_WIDGET (panel_box), "panel_box");

    PanelApplicationsMenu *apps_menu
        = g_object_new (PANEL_TYPE_APPLICATIONS_MENU, NULL);

    gtk_box_pack_start (
        panel_box,
        GTK_WIDGET (panel_applications_menu_get_launcher_button (apps_menu)),
        FALSE, FALSE, 0);

    hide_applications_menu (apps_menu);

    PanelTaskbar *panel_taskbar = panel_taskbar_init ();

    self->taskbar = panel_taskbar;

    gtk_box_pack_start (panel_box, GTK_WIDGET (panel_taskbar->taskbar_box),
                        FALSE, FALSE, 0);

    GTask *task = g_task_new (gtk_window, NULL, NULL, NULL);
    g_task_set_task_data (task, panel_taskbar, NULL);
    g_task_run_in_thread (task, (GTaskThreadFunc)panel_taskbar_run_wrap);
    
    PanelTray *panel_tray = panel_tray_new ();

    gtk_box_pack_end (panel_box, GTK_WIDGET (panel_tray->box), FALSE, FALSE,
                      0);

    gtk_container_add (GTK_CONTAINER (gtk_window), GTK_WIDGET (panel_box));

    gtk_widget_show_all (GTK_WIDGET (panel_box));
    gtk_widget_show_all (GTK_WIDGET (gtk_window));

    gtk_widget_set_app_paintable (GTK_WIDGET (gtk_window), TRUE);
    gtk_widget_show (GTK_WIDGET (gtk_window));

    GdkWindow *win = gtk_widget_get_window (GTK_WIDGET (gtk_window));

    GSettings *settings = g_settings_new ("org.gnome.desktop.interface");
    GSettings *bg_settings = g_settings_new ("org.gnome.desktop.background");

    char *bg = NULL;
    if (!strcmp (g_settings_get_string (settings, "color-scheme"),
                 "prefer-dark")) {
        bg = g_settings_get_string (bg_settings, "picture-uri-dark");
        self->dark_mode = TRUE;
    } else {
        bg = g_settings_get_string (bg_settings, "picture-uri");
        self->dark_mode = FALSE;
    }

    GdkPixbuf *pbuf = NULL;

    // "/usr/share/backgrounds/gnome/drool-d.svg"
    if (bg != NULL) {
        pbuf = gdk_pixbuf_new_from_file_at_size (bg + 7, 2560, -1, NULL);
    } else {
        pbuf = gdk_pixbuf_new_from_file_at_size (
            "/usr/share/backgrounds/gnome/drool-d.svg", 2560, -1, NULL);
    }

    int scale_factor = gdk_monitor_get_scale_factor (
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

    self->blurred = scaled;

    panel_applications_menu_set_bg (apps_menu, self->blurred);

    g_signal_connect (gtk_window, "draw", G_CALLBACK (expose_draw), self);
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
