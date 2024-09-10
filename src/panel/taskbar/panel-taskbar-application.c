#include "panel-taskbar-application.h"
#include "panel-taskbar-toplevel-button.h"

static gboolean
button_click (GtkButton *button, PanelTaskbarApplication *self) {
    UNUSED (button);
    UNUSED (self);

    if (!self->toplevels || !self->toplevels->data) {
        // TODO: execute the app

        return FALSE;
    }

    PanelTaskbarToplevelButton *toplevel_old = self->toplevels->data;

    self->toplevels = g_list_remove (self->toplevels, toplevel_old);
    self->toplevels = g_list_append (self->toplevels, toplevel_old);

    PanelTaskbarToplevelButton *toplevel_new = self->toplevels->data;

    zwlr_foreign_toplevel_handle_v1_activate (toplevel_new->m_toplevel_handle,
                                              toplevel_new->m_seat);

    wl_display_roundtrip (self->taskbar->display);

    return FALSE;
}

static void
secondary_button_click (GtkGestureClick *click, gint n_press, gdouble x,
                        gdouble y, PanelTaskbarApplication *self) {
    UNUSED (click);
    UNUSED (n_press);
    UNUSED (x);
    UNUSED (y);

    gtk_popover_popup (self->popover);
}

static void
remove_app_finalize (GtkWidget *widget) {
    gtk_box_remove (GTK_BOX (gtk_widget_get_parent (widget)), widget);
}

static void
remove_app (PanelTaskbarApplication *self) {
    self->taskbar->applications
        = g_list_remove (self->taskbar->applications, self);

    free (self->id);
    self->id = NULL;
    g_list_free (self->toplevels);

    if (self->icon_path) {
        free (self->icon_path);
        self->icon_path = NULL;
    }

    GtkWidget *last_widget = GTK_WIDGET (self->taskbar_item_button);
    gtk_widget_set_sensitive (GTK_WIDGET (last_widget), FALSE);

    free (self);

    g_timeout_add_once (400, (GSourceOnceFunc)remove_app_finalize,
                        last_widget);
}

void
panel_taskbar_application_set_pinned (PanelTaskbarApplication *self,
                                      gboolean pinned) {
    self->pinned = pinned;

    if (!self->pinned && (!self->toplevels || !self->toplevels->data)) {
        remove_app (self);
    }
}

PanelTaskbarApplication *
panel_taskbar_application_new (char *id, PanelTaskbar *taskbar) {
    PanelTaskbarApplication *self = malloc (sizeof (PanelTaskbarApplication));

    self->pinned = false;

    self->id = g_strdup (id);
    self->icon_path = NULL;

    self->icon = NULL;

    self->toplevels = NULL;

    self->indicator = NULL;

    self->items_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

    self->taskbar_item_fixed = GTK_FIXED (gtk_fixed_new ());
    gtk_widget_set_size_request (GTK_WIDGET (self->taskbar_item_fixed), 48, 56);

    self->taskbar_item_button = GTK_BUTTON (gtk_button_new ());

    GtkGesture *right_click = gtk_gesture_click_new ();
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (right_click), 3);

    g_signal_connect (right_click, "pressed",
                      G_CALLBACK (secondary_button_click), self);
    gtk_widget_add_controller (GTK_WIDGET (self->taskbar_item_button),
                               GTK_EVENT_CONTROLLER (right_click));

    gtk_widget_set_name (GTK_WIDGET (self->taskbar_item_button),
                         "panel_button");

    gtk_button_set_child (self->taskbar_item_button,
                          GTK_WIDGET (self->taskbar_item_fixed));

    gtk_box_append (taskbar->taskbar_box,
                    GTK_WIDGET (self->taskbar_item_button));

    self->taskbar = taskbar;
    taskbar->applications = g_list_append (taskbar->applications, self);

    self->icon_path = get_icon_from_app_id (self->taskbar, self->id);

    GdkPixbuf *pbuf = NULL;
    if (self->icon_path)
        pbuf
            = gdk_pixbuf_new_from_file_at_size (self->icon_path, 48, 48, NULL);

    GtkImage *icon_2;

    if (pbuf) {
        self->icon = GTK_IMAGE (gtk_image_new_from_pixbuf (pbuf));
        icon_2 = GTK_IMAGE (gtk_image_new_from_pixbuf (pbuf));

        g_object_unref (pbuf);
    } else {
        self->icon = GTK_IMAGE (gtk_image_new ());
        icon_2 = GTK_IMAGE (gtk_image_new ());
    }

    gtk_widget_set_size_request (GTK_WIDGET (self->icon), 48, 48);
    gtk_widget_set_size_request (GTK_WIDGET (icon_2), 48, 48);

    gtk_widget_set_name (GTK_WIDGET (icon_2), "panel_button_reflection");

    self->indicator = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name (self->indicator, "indicator");
    gtk_widget_set_size_request (self->indicator, 6, 6);
    gtk_widget_set_halign (self->indicator, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (self->indicator, GTK_ALIGN_CENTER);

    gtk_fixed_put (self->taskbar_item_fixed, GTK_WIDGET (icon_2), 0, 0);
    gtk_fixed_put (self->taskbar_item_fixed, GTK_WIDGET (self->icon), 0, 0);
    gtk_fixed_put (self->taskbar_item_fixed, GTK_WIDGET (self->indicator), 21, 48);
    
    g_signal_connect (self->taskbar_item_button, "clicked",
                      G_CALLBACK (button_click), self);

    self->popover = GTK_POPOVER (gtk_popover_new ());

    gtk_widget_set_name (GTK_WIDGET (self->popover), "panel_popover");

    gtk_popover_set_child (self->popover, GTK_WIDGET (self->items_box));

    gtk_fixed_put (self->taskbar_item_fixed, GTK_WIDGET (self->popover), 0, 0);

    gtk_popover_present (self->popover);

    return self;
}

void
panel_taskbar_application_add_toplevel (PanelTaskbarApplication *self,
                                        PanelTaskbarToplevelButton *toplevel) {
    if (!g_list_find (self->toplevels, toplevel)) {
        self->toplevels = g_list_append (self->toplevels, toplevel);

        gtk_box_append (self->items_box, toplevel->rendered);
    }

    gtk_widget_set_opacity (self->indicator, 1.0);
}

void
panel_taskbar_application_remove_toplevel (
    PanelTaskbarApplication *self, PanelTaskbarToplevelButton *toplevel) {
    if (self->toplevels && g_list_find (self->toplevels, toplevel)) {
        gtk_box_remove (self->items_box, toplevel->rendered);

        self->toplevels = g_list_remove (self->toplevels, toplevel);
    }

    if (!self->toplevels || !self->toplevels->data) {
        if (self->pinned) {
            gtk_widget_set_opacity (self->indicator, 0.0);
        } else {
            remove_app (self);
        }
    }
}