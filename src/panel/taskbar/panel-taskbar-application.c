#include "panel-taskbar-application.h"
#include "panel-taskbar-toplevel-button.h"

static gboolean
button_click (GtkButton *button, PanelTaskbarApplicationRendered *self) {
    UNUSED (button);
    UNUSED (self);

    if (!self->app->toplevels || !self->app->toplevels->data) {
        if (self->app->exec) {
            if (!strcmp (self->app->exec, DASHBOARD_LAUNCH_STR)) {
                show_applications_menu (self->win->apps_menu);
            } else {
                size_t len = strlen (self->app->exec) + strlen (" &") + 1;
                char *new_exec = malloc (len);
                snprintf (new_exec, len, "%s &", self->app->exec);

                system (new_exec);

                free (new_exec);
            }
        }

        return FALSE;
    }

    PanelTaskbarToplevelButton *toplevel_old = self->app->toplevels->data;

    self->app->toplevels = g_list_remove (self->app->toplevels, toplevel_old);
    self->app->toplevels = g_list_append (self->app->toplevels, toplevel_old);

    PanelTaskbarToplevelButton *toplevel_new = self->app->toplevels->data;

    zwlr_foreign_toplevel_handle_v1_activate (toplevel_new->m_toplevel_handle,
                                              toplevel_new->m_seat);

    wl_display_roundtrip (self->app->taskbar->display);

    return FALSE;
}

static void
secondary_button_click (GtkGestureClick *click, gint n_press, gdouble x,
                        gdouble y, PanelTaskbarApplicationRendered *self) {
    UNUSED (click);
    UNUSED (n_press);
    UNUSED (x);
    UNUSED (y);

    GtkWidget *parent
        = gtk_widget_get_parent (GTK_WIDGET (self->app->popover));

    if (parent) {
        g_object_ref (self->app->popover);

        gtk_fixed_remove (GTK_FIXED (parent), GTK_WIDGET (self->app->popover));
    }

    gtk_fixed_put (GTK_FIXED (gtk_button_get_child (self->button)),
                   GTK_WIDGET (self->app->popover), 0, 0);

    if (parent) {
        g_object_unref (self->app->popover);
    }

    gtk_popover_popup (self->app->popover);
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

    if (self->rendered_buttons) {
        for (int i = 0; self->rendered_buttons[i]; i++) {
            GtkWidget *last_widget
                = GTK_WIDGET (self->rendered_buttons[i]->button);
            gtk_widget_set_sensitive (GTK_WIDGET (last_widget), FALSE);

            g_timeout_add_once (400, (GSourceOnceFunc)remove_app_finalize,
                                last_widget);

            free (self->rendered_buttons[i]);
        }
    }

    free (self);
}

void
panel_taskbar_application_set_pinned (PanelTaskbarApplication *self,
                                      gboolean pinned) {
    self->pinned = pinned;

    if (!self->pinned && (!self->toplevels || !self->toplevels->data)) {
        remove_app (self);
    }
}

static PanelTaskbarApplicationRendered *
panel_taskbar_application_render_button (PanelTaskbarWindow *win,
                                         PanelTaskbarApplication *self) {
    PanelTaskbarApplicationRendered *return_val
        = malloc (sizeof (PanelTaskbarApplicationRendered));

    GtkButton *taskbar_item_button = GTK_BUTTON (gtk_button_new ());

    GtkGesture *right_click = gtk_gesture_click_new ();
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (right_click), 3);

    g_signal_connect (right_click, "pressed",
                      G_CALLBACK (secondary_button_click), return_val);
    gtk_widget_add_controller (GTK_WIDGET (taskbar_item_button),
                               GTK_EVENT_CONTROLLER (right_click));

    gtk_widget_add_css_class (GTK_WIDGET (taskbar_item_button), "taskbar_button");

    GtkFixed *taskbar_item_fixed = GTK_FIXED (gtk_fixed_new ());

    gtk_button_set_child (taskbar_item_button,
                          GTK_WIDGET (taskbar_item_fixed));

    gtk_box_append (win->taskbar_box, GTK_WIDGET (taskbar_item_button));

    GdkPixbuf *pbuf = NULL;
    if (self->icon_path)
        pbuf
            = gdk_pixbuf_new_from_file_at_size (self->icon_path, 48, 48, NULL);

    GtkImage *icon, *icon_reflection;

    bool should_reflect = (win->style == TASKBAR_STYLE_THREE_D_DOCK);
    bool show_indicator = (win->style != TASKBAR_STYLE_TRAY);

    if (pbuf) {
        icon = GTK_IMAGE (gtk_image_new_from_pixbuf (pbuf));

        if (should_reflect)
            icon_reflection = GTK_IMAGE (gtk_image_new_from_pixbuf (pbuf));

        g_object_unref (pbuf);
    } else {
        icon = GTK_IMAGE (gtk_image_new ());

        if (should_reflect)
            icon_reflection = GTK_IMAGE (gtk_image_new ());
    }

    // gtk_widget_set_size_request (GTK_WIDGET (icon), 48, 48);
    gtk_widget_add_css_class (GTK_WIDGET (icon), "taskbar_button_icon");

    if (should_reflect) {
        // gtk_widget_set_size_request (GTK_WIDGET (icon_reflection), 48, 48);
        gtk_widget_add_css_class (GTK_WIDGET (icon_reflection),
                             "taskbar_button_reflection");

        gtk_fixed_put (taskbar_item_fixed, GTK_WIDGET (icon_reflection), 0, 0);
    }

    if (show_indicator) {
        GtkWidget *indicator = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_add_css_class (indicator, "indicator");
        gtk_widget_set_size_request (indicator, 6, 6);
        gtk_widget_set_halign (indicator, GTK_ALIGN_CENTER);
        gtk_widget_set_valign (indicator, GTK_ALIGN_CENTER);
        gtk_fixed_put (taskbar_item_fixed, GTK_WIDGET (indicator), 21, 48);

        return_val->indicator = indicator;
    } else {
        return_val->indicator = NULL;
    }

    gtk_fixed_put (taskbar_item_fixed, GTK_WIDGET (icon), 0, 0);

    g_signal_connect (taskbar_item_button, "clicked",
                      G_CALLBACK (button_click), return_val);

    return_val->app = self;
    return_val->button = taskbar_item_button;
    return_val->win = win;

    return return_val;
}

void
panel_taskbar_application_update_monitors (PanelTaskbarApplication *self) {
    if (self->rendered_buttons) {
        for (size_t i = 0; self->rendered_buttons[i]; i++) {
            PanelTaskbarApplicationRendered *rendered
                = self->rendered_buttons[i];

            GtkBox *parent = GTK_BOX (
                gtk_widget_get_parent (GTK_WIDGET (rendered->button)));
            if (parent)
                gtk_box_remove (parent, GTK_WIDGET (rendered->button));

            free (rendered);
        }

        free (self->rendered_buttons);
    }

    if (!self->taskbar->windows) {
        self->rendered_buttons = malloc (sizeof (PanelTaskbarWindow *));
        self->rendered_buttons[0] = NULL;
        return;
    }

    size_t n_taskbars;

    for (n_taskbars = 0; self->taskbar->windows[n_taskbars]; n_taskbars++)
        ;

    self->rendered_buttons
        = malloc ((n_taskbars + 1) * sizeof (PanelTaskbarWindow *));

    for (int i = 0; self->taskbar->windows[i]; i++) {
        PanelTaskbarWindow *window = self->taskbar->windows[i];

        // TODO: check if the app should be shown on this window.
        bool should_show = true;

        if (should_show) {
            self->rendered_buttons[i]
                = panel_taskbar_application_render_button (window, self);
        } else {
            self->rendered_buttons[i]
                = malloc (sizeof (PanelTaskbarApplicationRendered));

            self->rendered_buttons[i]->button = NULL;
            self->rendered_buttons[i]->indicator = NULL;
            self->rendered_buttons[i]->app = self;
            self->rendered_buttons[i]->win = window;
        }
    }

    self->rendered_buttons[n_taskbars] = NULL;
}

PanelTaskbarApplication *
panel_taskbar_application_new (char *id, PanelTaskbar *taskbar) {
    PanelTaskbarApplication *self = malloc (sizeof (PanelTaskbarApplication));

    self->pinned = false;

    self->id = g_strdup (id);
    self->icon_path = NULL;
    self->exec = NULL;
    self->toplevels = NULL;
    self->rendered_buttons = NULL;

    self->items_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

    self->taskbar = taskbar;
    taskbar->applications = g_list_append (taskbar->applications, self);

    self->icon_path = get_icon_from_app_id (self->taskbar, self->id);

    self->popover = GTK_POPOVER (gtk_popover_new ());

    gtk_popover_present (self->popover);

    gtk_widget_add_css_class (GTK_WIDGET (self->popover), "taskbar_popover");

    gtk_popover_set_child (self->popover, GTK_WIDGET (self->items_box));

    panel_taskbar_application_update_monitors (self);

    /* gtk_fixed_put (self->taskbar_item_fixed, GTK_WIDGET (self->popover), 0,
    0);

    gtk_popover_present (self->popover); */

    return self;
}

void
panel_taskbar_application_add_toplevel (PanelTaskbarApplication *self,
                                        PanelTaskbarToplevelButton *toplevel) {
    if (!g_list_find (self->toplevels, toplevel)) {
        self->toplevels = g_list_append (self->toplevels, toplevel);

        gtk_box_append (self->items_box, toplevel->rendered);
    }

    if (self->rendered_buttons) {
        for (int i = 0; self->rendered_buttons[i]; i++) {
            if (self->rendered_buttons[i]->indicator) {
                gtk_widget_set_opacity (self->rendered_buttons[i]->indicator,
                                        1.0);
            }
        }
    }
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
            if (self->rendered_buttons) {
                for (int i = 0; self->rendered_buttons[i]; i++) {
                    gtk_widget_set_opacity (
                        self->rendered_buttons[i]->indicator, 0.0);
                }
            }
        } else {
            remove_app (self);
        }
    }
}