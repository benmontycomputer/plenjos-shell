#include "panel-taskbar.h"
#include "tray/panel-tray.h"

static void draw_taskbar_3d_dock (GtkDrawingArea *drawing_area, cairo_t *cr,
                                  int wi, int hi, PanelTaskbarWindow *self);

PanelTaskbarWindow *
panel_taskbar_window_new (PanelTaskbar *taskbar, GdkMonitor *monitor) {
    PanelTaskbarWindow *self = malloc (sizeof (PanelTaskbarWindow));

    self->taskbar = taskbar;
    self->monitor = monitor;

    self->style = TASKBAR_STYLE_THREE_D_DOCK;

    char *style_str
        = g_settings_get_string (taskbar->panel->panel_settings, "style");

    if (style_str) {
        if (!strcmp (style_str, "3D-Dock")) {
            self->style = TASKBAR_STYLE_THREE_D_DOCK;
        } else if (!strcmp (style_str, "2D-Dock")) {
            self->style = TASKBAR_STYLE_DOCK;
        } else if (!strcmp (style_str, "2D-Panel")) {
            self->style = TASKBAR_STYLE_PANEL;
        } else if (!strcmp (style_str, "Tray")) {
            self->style = TASKBAR_STYLE_TRAY;
        } else if (!strcmp (style_str, "Invisible")) {
            self->style = TASKBAR_STYLE_INVISIBLE;
        }

        free (style_str);
    }

    self->bg_primary.red = 0;
    self->bg_primary.green = 0;
    self->bg_primary.blue = 0;
    self->bg_primary.alpha = 1;

    char *bg_primary_str = g_settings_get_string (
        taskbar->panel->panel_settings, "background-color-top");
    if (bg_primary_str) {
        gdk_rgba_parse (&self->bg_primary, bg_primary_str);

        free (bg_primary_str);
    }

    self->bg_primary_bottom.red = 0;
    self->bg_primary_bottom.green = 0;
    self->bg_primary_bottom.blue = 0;
    self->bg_primary_bottom.alpha = 1;

    char *bg_primary_bottom_str = g_settings_get_string (
        taskbar->panel->panel_settings, "background-color-bottom");
    if (bg_primary_bottom_str) {
        gdk_rgba_parse (&self->bg_primary_bottom, bg_primary_bottom_str);

        free (bg_primary_bottom_str);
    }

    self->border_primary.red = 0;
    self->border_primary.green = 0;
    self->border_primary.blue = 0;
    self->border_primary.alpha = 1;

    char *border_primary_str = g_settings_get_string (
        taskbar->panel->panel_settings, "border-color");
    if (border_primary_str) {
        gdk_rgba_parse (&self->border_primary, border_primary_str);

        free (border_primary_str);
    }

    // Start actual taskbar stuff

    self->apps_menu = g_object_new (PANEL_TYPE_APPLICATIONS_MENU, NULL);

    hide_applications_menu (self->apps_menu);

    self->taskbar_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    // gtk_widget_add_css_class (GTK_WIDGET (self->taskbar_box),
    // "taskbar_box"); panel_applications_menu_insert_launcher_button
    // (apps_menu, self->taskbar_box);
    panel_applications_menu_set_monitor (self->apps_menu, monitor);

    GtkBox *box2 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
    gtk_widget_set_halign (GTK_WIDGET (box2), GTK_ALIGN_CENTER);

    GtkDrawingArea *da = GTK_DRAWING_AREA (gtk_drawing_area_new ());

    gtk_widget_set_size_request (GTK_WIDGET (da), 0, 64);

    gtk_widget_set_halign (GTK_WIDGET (da), GTK_ALIGN_FILL);
    gtk_widget_set_hexpand (GTK_WIDGET (da), TRUE);

    gtk_widget_set_margin_bottom (GTK_WIDGET (da), -72);

    gtk_widget_set_hexpand (GTK_WIDGET (da), TRUE);
    gtk_widget_set_vexpand (GTK_WIDGET (da), TRUE);

    // gtk_drawing_area_set_draw_func (da,
    // (GtkDrawingAreaDrawFunc)draw_taskbar,
    //                                 self, NULL);

    if (self->style == TASKBAR_STYLE_THREE_D_DOCK
        || self->style == TASKBAR_STYLE_DOCK
        || self->style == TASKBAR_STYLE_PANEL) {
        GtkWindow *gtk_window
            = GTK_WINDOW (gtk_application_window_new (taskbar->panel->app));
        self->gtk_window = gtk_window;

        gtk_layer_init_for_window (gtk_window);

        gtk_layer_set_layer (gtk_window, GTK_LAYER_SHELL_LAYER_TOP);
        gtk_layer_auto_exclusive_zone_enable (gtk_window);
        gtk_layer_set_monitor (self->gtk_window, monitor);

        static const gboolean anchors[] = { TRUE, TRUE, FALSE, TRUE };
        for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++) {
            gtk_layer_set_anchor (gtk_window, i, anchors[i]);
        }

        gtk_layer_set_margin (gtk_window, GTK_LAYER_SHELL_EDGE_BOTTOM, 1);
        gtk_widget_set_margin_bottom (GTK_WIDGET (da), -71);

        // gtk_widget_set_size_request (GTK_WIDGET (gtk_window), 480, 56);
        gtk_widget_add_css_class (GTK_WIDGET (gtk_window), "panel_window");

        gtk_widget_add_css_class (GTK_WIDGET (self->taskbar_box),
                                  "taskbar_box_full");
        gtk_widget_set_halign (GTK_WIDGET (self->taskbar_box),
                               GTK_ALIGN_CENTER);

        gtk_box_append (box2, GTK_WIDGET (self->taskbar_box));

        gtk_window_set_child (gtk_window, GTK_WIDGET (box2));

        gtk_window_present (gtk_window);
    } else if (self->style == TASKBAR_STYLE_TRAY) {
        gtk_widget_add_css_class (GTK_WIDGET (self->taskbar_box),
                                  "menu_bar_item");

        gtk_widget_add_css_class (GTK_WIDGET (self->taskbar_box),
                                  "taskbar_box_slim");

        for (int i = 0; ((PanelTray *)taskbar->panel->tray)->windows[i]; i++) {
            PanelTrayWindow *tray_win
                = ((PanelTray *)taskbar->panel->tray)->windows[i];

            if (tray_win->monitor == monitor) {
                gtk_box_append (tray_win->tray_start_box,
                                GTK_WIDGET (self->taskbar_box));

                break;
            }
        }
    }

    switch (self->style) {
    case TASKBAR_STYLE_THREE_D_DOCK:
        GtkDrawingArea *da = GTK_DRAWING_AREA (gtk_drawing_area_new ());

        gtk_widget_set_size_request (GTK_WIDGET (da), 0, 64);

        gtk_widget_set_halign (GTK_WIDGET (da), GTK_ALIGN_FILL);
        gtk_widget_set_hexpand (GTK_WIDGET (da), TRUE);

        gtk_widget_set_margin_bottom (GTK_WIDGET (da), -72);

        gtk_widget_set_hexpand (GTK_WIDGET (da), TRUE);
        gtk_widget_set_vexpand (GTK_WIDGET (da), TRUE);

        gtk_drawing_area_set_draw_func (
            da, (GtkDrawingAreaDrawFunc)draw_taskbar_3d_dock, self, NULL);

        gtk_box_prepend (box2, GTK_WIDGET (da));
        break;
    case TASKBAR_STYLE_DOCK:
        gtk_widget_add_css_class (GTK_WIDGET (self->taskbar_box),
                                  "taskbar_box_2d_dock");
    default:
        break;
    }

    return self;
}

static void
draw_taskbar_3d_dock (GtkDrawingArea *drawing_area, cairo_t *cr, int wi,
                      int hi, PanelTaskbarWindow *self) {
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

    cairo_set_source_rgba (
        cr, self->border_primary.red, self->border_primary.green,
        self->border_primary.blue, self->border_primary.alpha);

    cairo_stroke_preserve (cr);

    cairo_clip (cr);

    // cairo_set_source_rgba (cr, 0.7, 0.7, 0.7, 1.0);

    cairo_pattern_t *pattern
        = cairo_pattern_create_linear (0, y, 0, y + height - 4.0);

    // cairo_pattern_add_color_stop_rgba (pattern, 0.0, 0.3, 0.3, 0.3, 0.5);
    // cairo_pattern_add_color_stop_rgba (pattern, 1.0, 0.4, 0.4, 0.4, 0.5);

    cairo_pattern_add_color_stop_rgba (
        pattern, 0.0, self->bg_primary.red, self->bg_primary.green,
        self->bg_primary.blue, self->bg_primary.alpha);
    cairo_pattern_add_color_stop_rgba (
        pattern, 1.0, self->bg_primary_bottom.red,
        self->bg_primary_bottom.green, self->bg_primary_bottom.blue,
        self->bg_primary_bottom.alpha);

    cairo_set_source (cr, pattern);

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

    cairo_paint (cr);

    cairo_pattern_destroy (pattern);

    cairo_restore (cr);

    cairo_save (cr);

    // cairo_move_to (cr, x, y + height - 4.0);
    // cairo_line_to (cr, x + width, y + height - 4.0);
    cairo_new_path (cr);
    cairo_arc_negative (cr, x + 3.0, y + height - 7.0, 2.0, 180 * degrees,
                        90 * degrees);
    cairo_arc_negative (cr, x + width - 3.0, y + height - 7.0, 2.0,
                        90 * degrees, 0 * degrees);

    cairo_arc (cr, x + width - 3.0, y + height - 3.0, 2.0, 0, 90 * degrees);
    cairo_arc (cr, x + 3.0, y + height - 3.0, 2.0, 90 * degrees,
               180 * degrees);
    cairo_close_path (cr);

    cairo_set_source_rgba (
        cr, self->border_primary.red, self->border_primary.green,
        self->border_primary.blue, self->border_primary.alpha);

    cairo_stroke_preserve (cr);

    cairo_clip (cr);

    /*cairo_move_to (cr, x, y + height - 1.0);
    cairo_line_to (cr, x + width, y + height - 1.0);

    cairo_set_source_rgba (cr, 0.8, 0.8, 0.8, 1.0);

    cairo_set_line_width (cr, 2.0);
    cairo_stroke (cr);*/
    cairo_set_source_rgba (
        cr, self->bg_primary_bottom.red, self->bg_primary_bottom.green,
        self->bg_primary_bottom.blue, self->bg_primary_bottom.alpha);

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

    cairo_paint (cr);

    cairo_restore (cr);
}