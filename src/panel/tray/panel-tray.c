#include "panel-tray.h"

static void
show_control_center (GtkButton *button, PanelTray *self) {
    UNUSED (button);

    gtk_stack_set_visible_child_name (self->stack, "Control Center");

    panel_tray_menu_toggle_show (self->menu);
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
        gtk_widget_set_visible (GTK_WIDGET (self->back_button), FALSE);
    } else {
        gtk_widget_set_visible (GTK_WIDGET (self->back_button), TRUE);
    }
}

PanelTray *
panel_tray_new (gpointer panel_ptr) {
    PanelTray *self = malloc (sizeof (PanelTray));

    self->stack = GTK_STACK (gtk_stack_new ());

    gtk_stack_set_transition_type (self->stack,
                                   GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);

    self->control_center_grid = GTK_GRID (gtk_grid_new ());

    gtk_widget_set_name (GTK_WIDGET (self->control_center_grid),
                         "control_center_grid");

    gtk_widget_set_hexpand (GTK_WIDGET (self->control_center_grid), TRUE);
    gtk_widget_set_vexpand (GTK_WIDGET (self->control_center_grid), TRUE);

    gtk_grid_set_row_homogeneous (self->control_center_grid, FALSE);
    gtk_grid_set_column_homogeneous (self->control_center_grid, TRUE);

    gtk_grid_set_row_spacing (self->control_center_grid, 10);
    gtk_grid_set_column_spacing (self->control_center_grid, 10);

    // Icon size DND
    self->control_center_button
        = GTK_BUTTON (gtk_button_new ());
    
    GtkImage *img = GTK_IMAGE (gtk_image_new_from_icon_name ("tweaks-app"));
    gtk_button_set_child (self->control_center_button, GTK_WIDGET (img));
    gtk_widget_set_valign (GTK_WIDGET (self->control_center_button), GTK_ALIGN_START);

    gtk_widget_set_name (GTK_WIDGET (self->control_center_button),
                         "panel_button");

    g_signal_connect (self->control_center_button, "clicked",
                      G_CALLBACK (show_control_center), self);

    gtk_stack_add_titled (self->stack, GTK_WIDGET (self->control_center_grid),
                          "Control Center", "Control Center");

    self->media_control = media_control_new ();

    self->menu = panel_tray_menu_new (panel_ptr);

    self->back_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8));

    // Icon size LARGE_TOOLBAR
    self->back_button
        = GTK_BUTTON (gtk_button_new_from_icon_name ("go-previous"));

    gtk_widget_set_name (GTK_WIDGET (self->back_button),
                         "control_center_back_button");

    g_signal_connect (self->back_button, "clicked", G_CALLBACK (go_back),
                      self);

    self->back_label = GTK_LABEL (gtk_label_new ("Back"));

    gtk_widget_set_name (GTK_WIDGET (self->back_label),
                         "control_center_back_label");

    gtk_box_append (self->back_box, GTK_WIDGET (self->back_button));
    gtk_box_append (self->back_box, GTK_WIDGET (self->back_label));

    g_signal_connect (self->stack, "notify::visible-child",
                      G_CALLBACK (stack_child_changed), self);

    gtk_box_append (self->menu->box, GTK_WIDGET (self->back_box));
    gtk_box_append (self->menu->box, GTK_WIDGET (self->stack));

    gtk_widget_set_visible (GTK_WIDGET (self->back_button), FALSE);

    self->network_button = network_button_new (self->stack);
    self->audio_button = audio_button_new (self->stack);
    self->power_button = power_button_new (self->stack);

    self->box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));

    self->clock = clock_new ();

    g_timeout_add (500, (GSourceFunc)clock_update, self->clock);

    gtk_grid_attach (self->control_center_grid,
                     GTK_WIDGET (self->media_control->box), 0, 0, 3, 1);

    gtk_grid_attach (self->control_center_grid,
                     GTK_WIDGET (self->network_button->button), 0, 1, 1, 1);
    gtk_grid_attach (self->control_center_grid,
                     GTK_WIDGET (self->audio_button->button), 1, 1, 1, 1);
    gtk_grid_attach (self->control_center_grid,
                     GTK_WIDGET (self->power_button->button), 2, 1, 1, 1);

    gtk_box_append (self->box, GTK_WIDGET (self->control_center_button));
    gtk_box_append (self->box, GTK_WIDGET (self->clock->label));

    return self;
}