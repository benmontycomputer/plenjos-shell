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