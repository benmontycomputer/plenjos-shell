#include "panel-tray.h"
#include "panel-common.h"

static void
show_control_center (GtkButton *button, PanelTray *self) {
    UNUSED (button);

    gtk_stack_set_visible_child_name (self->stack, "Control Center");

    // the ref and unref is needed so the widget isn't destroyed after being
    // removed from the box.
    g_object_ref (self->control_center_popover);

    if (gtk_widget_get_parent (GTK_WIDGET (self->control_center_popover))) {
        gtk_box_remove (GTK_BOX (gtk_widget_get_parent (
                            GTK_WIDGET (self->control_center_popover))),
                        GTK_WIDGET (self->control_center_popover));
    }
    gtk_box_append (GTK_BOX (gtk_widget_get_parent (GTK_WIDGET (button))),
                    GTK_WIDGET (self->control_center_popover));

    g_object_unref (self->control_center_popover);

    // panel_tray_menu_toggle_show (self->menu);
    gtk_popover_popup (self->control_center_popover);
}

static void
go_back (GtkButton *button, PanelTray *self) {
    UNUSED (button);

    gtk_stack_set_visible_child_name (self->stack, "Control Center");
}

static void
stack_child_changed (GtkStack *stack, GtkWidget *child, PanelTray *self) {
    UNUSED (child);

    gtk_label_set_text (self->back_label,
                        gtk_stack_get_visible_child_name (stack));

    if (strcmp (gtk_stack_get_visible_child_name (stack), "Control Center")
        == 0) {
        gtk_widget_set_visible (GTK_WIDGET (self->back_box), FALSE);
    } else {
        gtk_widget_set_visible (GTK_WIDGET (self->back_box), TRUE);
    }
}

void
panel_tray_update_monitors (PanelTray *self) {
    // the ref and unref is needed so the widget isn't destroyed after being
    // removed from the box.
    g_object_ref (self->control_center_popover);

    if (gtk_widget_get_parent (GTK_WIDGET (self->control_center_popover))) {
        gtk_box_remove (GTK_BOX (gtk_widget_get_parent (
                            GTK_WIDGET (self->control_center_popover))),
                        GTK_WIDGET (self->control_center_popover));
    }

    if (self->windows) {
        for (size_t i = 0; self->windows[i]; i++) {
            PanelTrayWindow *win = self->windows[i];

            gtk_window_close (win->gtk_window);

            free (win);
        }
    }

    size_t n_monitors
        = g_list_model_get_n_items (((Panel *)self->panel)->monitors);

    self->windows = malloc ((n_monitors + 1) * sizeof (PanelTrayWindow *));

    for (size_t i = 0; i < n_monitors; i++) {
        PanelTrayWindow *window = malloc (sizeof (PanelTrayWindow));

        window->gtk_window = GTK_WINDOW (
            gtk_application_window_new (((Panel *)self->panel)->app));

        // Before the window is first realized, set it up to be a layer surface
        gtk_layer_init_for_window (window->gtk_window);

        // Order below normal windows
        gtk_layer_set_layer (window->gtk_window, GTK_LAYER_SHELL_LAYER_TOP);

        // Push other windows out of the way
        gtk_layer_auto_exclusive_zone_enable (window->gtk_window);

        GdkMonitor *monitor = GDK_MONITOR (
            g_list_model_get_item (((Panel *)self->panel)->monitors, i));

        gtk_layer_set_monitor (window->gtk_window, monitor);
        window->monitor = monitor;

        static const gboolean anchors_2[] = { TRUE, TRUE, TRUE, FALSE };
        for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++) {
            gtk_layer_set_anchor (window->gtk_window, i, anchors_2[i]);
        }

        gtk_widget_set_size_request (GTK_WIDGET (window->gtk_window), 480, 32);

        gtk_widget_set_name (GTK_WIDGET (window->gtk_window),
                             "panel_topbar_window");

        window->tray_box = GTK_CENTER_BOX (gtk_center_box_new ());

        gtk_widget_set_hexpand (GTK_WIDGET (window->tray_box), TRUE);

        gtk_widget_set_name (GTK_WIDGET (window->tray_box), "menu_bar_box");

        gtk_window_set_child (window->gtk_window,
                              GTK_WIDGET (window->tray_box));
        gtk_widget_show (GTK_WIDGET (window->gtk_window));

        window->control_center_button = GTK_BUTTON (gtk_button_new ());

        GtkBox *control_center_button_box
            = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8));

        gtk_box_append (control_center_button_box,
                        gtk_label_new ("Control Center"));
        gtk_box_append (control_center_button_box,
                        gtk_image_new_from_icon_name ("tweaks-app-symbolic"));

        gtk_button_set_child (window->control_center_button,
                              GTK_WIDGET (control_center_button_box));

        gtk_widget_set_name (GTK_WIDGET (window->control_center_button),
                             "menu_bar_button");

        g_signal_connect (window->control_center_button, "clicked",
                          G_CALLBACK (show_control_center), self);

        window->clock = clock_new ();

        g_timeout_add (500, (GSourceFunc)clock_update, window->clock);

        window->workspaces_box
            = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_add_css_class (GTK_WIDGET (window->workspaces_box),
                                  "panel_top_bar_item");

        window->tray_end_box
            = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_add_css_class (GTK_WIDGET (window->tray_end_box),
                                  "panel_top_bar_item");

        gtk_box_append (window->tray_end_box,
                        GTK_WIDGET (window->control_center_button));

        gtk_center_box_set_start_widget (window->tray_box,
                                         GTK_WIDGET (window->workspaces_box));
        gtk_center_box_set_center_widget (window->tray_box,
                                          GTK_WIDGET (window->clock->label));
        gtk_center_box_set_end_widget (window->tray_box,
                                       GTK_WIDGET (window->tray_end_box));

        self->windows[i] = window;
    }

    self->windows[n_monitors] = NULL;

    g_object_unref (self->control_center_popover);
}

PanelTray *
panel_tray_new (gpointer *panel) {
    PanelTray *self = malloc (sizeof (PanelTray));

    self->panel = panel;

    self->stack = GTK_STACK (gtk_stack_new ());

    self->windows = NULL;

    gtk_stack_set_interpolate_size (self->stack, TRUE);
    gtk_stack_set_hhomogeneous (self->stack, TRUE);
    gtk_stack_set_vhomogeneous (self->stack, FALSE);

    gtk_stack_set_transition_type (self->stack,
                                   GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration (self->stack, 500);

    self->control_center_grid = GTK_GRID (gtk_grid_new ());

    gtk_widget_set_name (GTK_WIDGET (self->control_center_grid),
                         "control_center_grid");

    gtk_grid_set_row_homogeneous (self->control_center_grid, FALSE);
    gtk_grid_set_column_homogeneous (self->control_center_grid, TRUE);

    gtk_grid_set_row_spacing (self->control_center_grid, 12);
    gtk_grid_set_column_spacing (self->control_center_grid, 12);

    // Icon size DND
    // self->control_center_button = GTK_BUTTON (gtk_button_new ());

    /* GtkBox *control_center_button_box
        = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8));

    gtk_box_append (control_center_button_box,
                    gtk_label_new ("Control Center"));
    gtk_box_append (control_center_button_box,
                    gtk_image_new_from_icon_name ("tweaks-app-symbolic"));

    gtk_button_set_child (self->control_center_button,
                          GTK_WIDGET (control_center_button_box));

    gtk_widget_set_name (GTK_WIDGET (self->control_center_button),
                         "menu_bar_button");

    g_signal_connect (self->control_center_button, "clicked",
                      G_CALLBACK (show_control_center), self); */

    gtk_stack_add_titled (self->stack, GTK_WIDGET (self->control_center_grid),
                          "Control Center", "Control Center");

    self->media_control = media_control_new ();

    // self->menu = panel_tray_menu_new (panel_ptr);
    self->control_center_popover = GTK_POPOVER (gtk_popover_new ());
    self->control_center_popover_box
        = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

    gtk_widget_set_name (GTK_WIDGET (self->control_center_popover),
                         "control_center_popover");

    gtk_popover_set_child (self->control_center_popover,
                           GTK_WIDGET (self->control_center_popover_box));

    gtk_popover_set_has_arrow (self->control_center_popover, FALSE);

    self->back_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8));

    gtk_widget_set_name (GTK_WIDGET (self->back_box),
                         "control_center_back_box");

    // Icon size LARGE_TOOLBAR
    self->back_button
        = GTK_BUTTON (gtk_button_new_from_icon_name ("go-previous"));

    gtk_widget_set_name (GTK_WIDGET (self->back_button),
                         "control_center_back_button");

    g_signal_connect (self->back_button, "clicked", G_CALLBACK (go_back),
                      self);

    self->back_label = GTK_LABEL (gtk_label_new ("Control Center"));

    gtk_widget_set_name (GTK_WIDGET (self->back_label),
                         "control_center_back_label");

    gtk_box_append (self->back_box, GTK_WIDGET (self->back_button));
    gtk_box_append (self->back_box, GTK_WIDGET (self->back_label));

    g_signal_connect (self->stack, "notify::visible-child",
                      G_CALLBACK (stack_child_changed), self);

    // gtk_box_append (self->menu->box, GTK_WIDGET (self->back_box));
    // gtk_box_append (self->menu->box, GTK_WIDGET (self->stack));
    gtk_box_append (self->control_center_popover_box,
                    GTK_WIDGET (self->back_box));
    gtk_box_append (self->control_center_popover_box,
                    GTK_WIDGET (self->stack));

    gtk_widget_set_visible (GTK_WIDGET (self->back_box), FALSE);

    self->network_button = network_button_new (self->stack);
    self->bluetooth_button = bluetooth_button_new (self->stack);
    self->audio_button = audio_button_new (self->stack);
    self->power_button = power_button_new (self->stack);

    /* self->clock = clock_new ();

    g_timeout_add (500, (GSourceFunc)clock_update, self->clock); */

    gtk_grid_attach (self->control_center_grid,
                     GTK_WIDGET (self->media_control->box), 0, 3, 4, 1);

    GtkBox *quick_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 8));
    GtkBox *dnd_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 8));
    GtkBox *audio_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 8));

    gtk_widget_set_name (GTK_WIDGET (quick_box), "control_center_card");
    gtk_widget_set_name (GTK_WIDGET (dnd_box), "control_center_card");

    gtk_box_append (quick_box, GTK_WIDGET (self->network_button->button));
    gtk_box_append (quick_box, GTK_WIDGET (self->bluetooth_button->button));
    gtk_box_append (quick_box, GTK_WIDGET (self->power_button->button));

    gtk_box_append (dnd_box, gtk_label_new ("Do Not Disturb"));

    gtk_box_append (audio_box, GTK_WIDGET (self->audio_button->button));

    gtk_grid_attach (self->control_center_grid, GTK_WIDGET (quick_box), 0, 0,
                     2, 2);

    gtk_grid_attach (self->control_center_grid, GTK_WIDGET (dnd_box), 2, 0, 2,
                     1);

    gtk_grid_attach (self->control_center_grid, GTK_WIDGET (audio_box), 0, 2,
                     4, 1);

    /* self->tray_box = GTK_CENTER_BOX (gtk_center_box_new ());

    self->tray_end_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));

    gtk_box_append (self->tray_end_box,
                    GTK_WIDGET (self->control_center_button));

    gtk_box_append (self->tray_end_box,
                    GTK_WIDGET (self->control_center_popover));

    gtk_center_box_set_center_widget (self->tray_box,
                                      GTK_WIDGET (self->clock->label));
    gtk_center_box_set_end_widget (self->tray_box,
                                   GTK_WIDGET (self->tray_end_box)); */

    return self;
}