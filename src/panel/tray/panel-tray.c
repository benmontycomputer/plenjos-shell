#include "panel-tray.h"

static void
show_control_center (GtkButton *button, PanelTray *self) {
    UNUSED (button);

    gtk_stack_set_visible_child_name (self->stack, "Control Center");

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

PanelTray *
panel_tray_new (gpointer panel_ptr) {
    UNUSED (panel_ptr);

    PanelTray *self = malloc (sizeof (PanelTray));

    self->stack = GTK_STACK (gtk_stack_new ());

    gtk_stack_set_interpolate_size (self->stack, TRUE);
    gtk_stack_set_hhomogeneous (self->stack, FALSE);
    gtk_stack_set_vhomogeneous (self->stack, FALSE);

    gtk_stack_set_transition_type (self->stack,
                                   GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration (self->stack, 500);

    self->control_center_grid = GTK_GRID (gtk_grid_new ());

    gtk_widget_set_name (GTK_WIDGET (self->control_center_grid),
                         "control_center_grid");

    gtk_widget_set_hexpand (GTK_WIDGET (self->control_center_grid), TRUE);
    gtk_widget_set_vexpand (GTK_WIDGET (self->control_center_grid), TRUE);

    gtk_grid_set_row_homogeneous (self->control_center_grid, FALSE);
    gtk_grid_set_column_homogeneous (self->control_center_grid, TRUE);

    gtk_grid_set_row_spacing (self->control_center_grid, 12);
    gtk_grid_set_column_spacing (self->control_center_grid, 12);

    // Icon size DND
    self->control_center_button = GTK_BUTTON (gtk_button_new ());

    GtkBox *control_center_button_box
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
                      G_CALLBACK (show_control_center), self);

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

    gtk_box_append (control_center_button_box,
                    GTK_WIDGET (self->control_center_popover));

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

    self->clock = clock_new ();

    g_timeout_add (500, (GSourceFunc)clock_update, self->clock);

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

    self->tray_box = GTK_CENTER_BOX (gtk_center_box_new ());

    self->tray_end_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));

    gtk_center_box_set_center_widget (self->tray_box,
                                      GTK_WIDGET (self->clock->label));
    gtk_center_box_set_end_widget (self->tray_box,
                                   GTK_WIDGET (self->control_center_button));

    return self;
}