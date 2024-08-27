#include "panel-tray-menu.h"

static void
draw_linear_shadow (cairo_t *cr, double x, double y, double dx, double dy,
                    double clip_x, double clip_y, double clip_w,
                    double clip_h) {
    cairo_pattern_t *pattern
        = cairo_pattern_create_linear (x, y, x + dx, y + dy);

    cairo_pattern_add_color_stop_rgba (pattern, 0, 0, 0, 0, 0.6);
    cairo_pattern_add_color_stop_rgba (pattern, 0.6, 0, 0, 0, 0.2);
    cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 0.0);

    cairo_save (cr);

    cairo_rectangle (cr, clip_x, clip_y, clip_w, clip_h);

    cairo_clip (cr);

    cairo_set_source (cr, pattern);
    cairo_mask (cr, pattern);

    cairo_pattern_destroy (pattern);

    cairo_restore (cr);
}

static void
draw_radial_shadow (cairo_t *cr, double x, double y, double r1, double r2,
                    int quadrant) {
    cairo_pattern_t *pattern
        = cairo_pattern_create_radial (x, y, r1, x, y, r2);

    cairo_pattern_add_color_stop_rgba (pattern, 0, 0, 0, 0, 0.6);
    cairo_pattern_add_color_stop_rgba (pattern, 0.6, 0, 0, 0, 0.2);
    cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 0.0);

    cairo_save (cr);

    if (quadrant == 1) {
        cairo_rectangle (cr, x, y - r2, r2, r2);
    } else if (quadrant == 2) {
        cairo_rectangle (cr, x, y, r2, r2);
    } else if (quadrant == 3) {
        cairo_rectangle (cr, x - r2, y, r2, r2);
    } else if (quadrant == 4) {
        cairo_rectangle (cr, x - r2, y - r2, r2, r2);
    }

    cairo_clip (cr);

    cairo_set_source (cr, pattern);
    cairo_mask (cr, pattern);

    cairo_pattern_destroy (pattern);

    cairo_restore (cr);
}

gboolean
expose_draw_tray_menu (GtkWidget *widget, cairo_t *cr, PanelTrayMenu *self) {
    UNUSED (widget);

    cairo_save (cr);

    // Adapted from
    // https://stackoverflow.com/questions/4183546/how-can-i-draw-image-with-rounded-corners-in-cairo-gtk
    double width, height, radius;
    int wi, hi;
    gtk_window_get_size (self->window, &wi, &hi);

    width = (double)wi;
    height = (double)hi;
    radius = 14;

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

    // Corner shadows (top right, bottom right, bottom left, top left)
    draw_radial_shadow (cr, x + width - radius, y + radius, radius,
                        radius + 6.0, 1);
    draw_radial_shadow (cr, x + width - radius, y + height - radius, radius,
                        radius + 6.0, 2);
    draw_radial_shadow (cr, x + radius, y + height - radius, radius,
                        radius + 6.0, 3);
    draw_radial_shadow (cr, x + radius, y + radius, radius, radius + 6.0, 4);

    // Bottom, top, left, right shadows
    draw_linear_shadow (cr, x + (width / 2), y + height, 0, 6.0, x + radius,
                        y + height, width - (2 * radius), 6.0);
    draw_linear_shadow (cr, x + (width / 2), y, 0, -6.0, x + radius, y - 6.0,
                        width - (2 * radius), 6.0);
    draw_linear_shadow (cr, x, y + (height / 2), -6.0, 0, x - 6.0, y + radius,
                        6.0, height - (2 * radius));
    draw_linear_shadow (cr, x + width, y + (height / 2), 6.0, 0, x + width,
                        y + radius, 6.0, height - (2 * radius));

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

    if (GDK_IS_PIXBUF (self->panel->blurred)) {
        gdk_cairo_set_source_pixbuf (cr, self->panel->blurred, -self->x, -self->y);
    } else {
        if (self->panel->supports_alpha) {
            cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.0);
        } else {
            cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
        }
    }

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);

    if (self->panel->dark_mode) {
        cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 0.5);
    } else {
        cairo_set_source_rgba (cr, 0.8, 0.8, 0.8, 0.5);
    }

    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

    cairo_paint (cr);

    cairo_restore (cr);

    return FALSE;
}

static gboolean
check_escape (GtkWidget *widget, GdkEventKey *event, PanelTrayMenu *self) {
    UNUSED (widget);

    if (self->visible && event->keyval == GDK_KEY_Escape) {
        panel_tray_menu_toggle_show (self);
        return TRUE;
    }

    return FALSE;
}

static gboolean
focus_out_event (GtkWidget *widget, GdkEventFocus *event,
                 PanelTrayMenu *self) {
    UNUSED (widget);
    UNUSED (event);

    if (self->visible)
        panel_tray_menu_toggle_show (self);

    return FALSE;
}

static gboolean
configure_event (GtkWidget *win, GdkEventConfigure *event,
                 PanelTrayMenu *self) {
    UNUSED (win);

    GdkWindow *gdk_window = gtk_widget_get_window (GTK_WIDGET (self->window));

    GdkRectangle geo;

    gdk_monitor_get_geometry (
        gdk_display_get_monitor_at_window (gdk_window_get_display (gdk_window),
                                           gdk_window),
        &geo);

    self->x = geo.width - 4 - event->width;
    self->y = geo.height - event->height;

    return FALSE;
}

PanelTrayMenu *
panel_tray_menu_new (Panel *panel) {
    PanelTrayMenu *self = malloc (sizeof (PanelTrayMenu));

    self->panel = panel;

    self->visible = FALSE;

    self->x = self->y = 0;

    self->window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));

    g_signal_connect (self->window, "configure-event",
                      G_CALLBACK (configure_event), self);

    // Before the window is first realized, set it up to be a layer surface
    gtk_layer_init_for_window (self->window);

    // Order below normal windows
    gtk_layer_set_layer (self->window, GTK_LAYER_SHELL_LAYER_OVERLAY);

    gtk_layer_set_keyboard_mode (
        self->window,
        GTK_LAYER_SHELL_KEYBOARD_MODE_ON_DEMAND); // NONE is default

    gtk_layer_set_anchor (self->window, GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
    gtk_layer_set_anchor (self->window, GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);

    gtk_widget_set_size_request (GTK_WIDGET (self->window), 400, -1);

    gtk_widget_set_app_paintable (GTK_WIDGET (self->window), TRUE);

    g_signal_connect (self->window, "draw", G_CALLBACK (expose_draw_tray_menu),
                      self);

    g_signal_connect (self->window, "key_press_event",
                      G_CALLBACK (check_escape), self);
    g_signal_connect (self->window, "focus-out-event",
                      G_CALLBACK (focus_out_event), self);

    self->box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

    gtk_container_add (GTK_CONTAINER (self->window), GTK_WIDGET (self->box));

    gtk_widget_set_name (GTK_WIDGET (self->window), "panel_tray_menu_window");
    gtk_widget_set_name (GTK_WIDGET (self->box), "panel_tray_menu_window_box");

    return self;
}

void
panel_tray_menu_toggle_show (PanelTrayMenu *self) {
    if (self->visible) {
        gtk_widget_hide (GTK_WIDGET (self->window));
    } else {
        gtk_widget_show (GTK_WIDGET (self->window));

        GdkWindow *gdk_window
            = gtk_widget_get_window (GTK_WIDGET (self->window));

        GdkRectangle geo;

        gdk_monitor_get_geometry (
            gdk_display_get_monitor_at_window (
                gdk_window_get_display (gdk_window), gdk_window),
            &geo);

        self->x = geo.width - 4 - gdk_window_get_width (gdk_window);
        self->y = geo.height - gdk_window_get_height (gdk_window);
    }

    self->visible = !self->visible;
}