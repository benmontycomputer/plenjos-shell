#include "panel-taskbar-toplevel-button.h"
#include "panel-taskbar-application.h"

static void
toplevel_handle_title (void *data,
                       struct zwlr_foreign_toplevel_handle_v1 *toplevel_handle,
                       const char *title) {
    PanelTaskbarToplevelButton *self = (PanelTaskbarToplevelButton *)data;

    UNUSED (toplevel_handle);

    self->m_title = g_strdup (title);

    panel_taskbar_toplevel_button_rerender (self, true, false);
}

// https://stackoverflow.com/questions/1726302
void
remove_spaces (char *s) {
    char *d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while ((*s++ = *d++));
}

char *
shorten_label (char *label) {
    if (strlen (label) > 24) {
        return g_strdup (label);
    } else {
        return NULL;
    }
}

char *
get_icon_from_app_id (PanelTaskbar *taskbar, char *id) {
    char *icon = NULL;

    char *current_theme
        = g_settings_get_string (taskbar->settings, "icon-theme");

    // Is this icon already loaded?
    // TODO: actually check if icon is already loaded

    if (icon == NULL) {
        GList *icon_exec_map = init_icon_exec_map ();

        GList *found = g_list_find_custom (icon_exec_map, id,
                                           (GCompareFunc)icon_exec_map_finder);

        if (found != NULL) {
            icon = suggested_icon_for_id (
                ((icon_exec_map_item *)found->data)->icon, 32, current_theme);
        } else {
            char *mod_id = g_strdup (id);

            for (size_t i = 0; mod_id[i]; i++) {
                mod_id[i] = tolower (mod_id[i]);
            }

            found = g_list_find_custom (icon_exec_map, mod_id,
                                        (GCompareFunc)icon_exec_map_finder);

            if (found != NULL) {
                icon = suggested_icon_for_id (
                    ((icon_exec_map_item *)found->data)->icon, 32,
                    current_theme);
            }

            free (mod_id);
        }

        g_list_free_full (icon_exec_map,
                          (GDestroyNotify)free_icon_exec_map_item);

        icon_exec_map = NULL;
    }
    if (icon == NULL) {
        icon = suggested_icon_for_id (id, 32, current_theme);
    }
    if (icon == NULL) {
        char *mod_id = g_strdup (id);

        remove_spaces (mod_id);
        icon = suggested_icon_for_id (mod_id, 32, current_theme);

        free (mod_id);
    }
    if (icon == NULL) {
        char *mod_id = g_strdup (id);

        for (size_t i = 0; mod_id[i]; i++) {
            mod_id[i] = tolower (mod_id[i]);
        }
        icon = suggested_icon_for_id (mod_id, 32, current_theme);

        free (mod_id);
    }
    if (icon == NULL) {
        // If id is in id.xx.xx format, extract first element
        size_t pos = 0;
        char *mod_id = g_strdup (id);

        while (mod_id[pos]) {
            if (mod_id[pos] == '.') {
                mod_id[pos] = '\0';
                break;
            }

            pos++;
        }

        icon = suggested_icon_for_id (mod_id, 32, current_theme);

        if (icon == NULL) {
            for (size_t i = 0; mod_id[i]; i++) {
                mod_id[i] = tolower (mod_id[i]);
            }

            icon = suggested_icon_for_id (mod_id, 32, current_theme);
        }

        free (mod_id);
    }
    if (icon == NULL) {
        // If id is in xx.xx.id format, extract last element
        size_t pos = 0;
        size_t i = 0;
        char *mod_id = g_strdup (id);

        while (mod_id[i]) {
            if (mod_id[i] == '.') {
                pos = i + 1;
            }

            i++;
        }

        icon = suggested_icon_for_id ((mod_id + pos), 32, current_theme);

        if (icon == NULL) {
            for (size_t i = 0; mod_id[i]; i++) {
                mod_id[i] = tolower (mod_id[i]);
            }

            icon = suggested_icon_for_id ((mod_id + pos), 32, current_theme);
        }

        free (mod_id);
    }
    if (icon == NULL) {
        icon = suggested_icon_for_id ("emblem-dialog-question", 32,
                                      current_theme);
    }

    free (current_theme);

    return icon;
}

gboolean
toplevel_handle_app_id_gtk (gpointer *data) {
    // TODO: run less code in here to make GUI more resposive
    PanelTaskbarToplevelButton *self = (PanelTaskbarToplevelButton *)data;

    if (!self->rendered)
        panel_taskbar_toplevel_button_gtk_run (self);

    if (self->m_application && self->m_application->id
        && strcmp (self->m_id, self->m_application->id)) {
        panel_taskbar_application_remove_toplevel (self->m_application, self);

        self->m_application = NULL;
    }

    if (!self->m_application) {
        GList *applications = self->m_taskbar->applications;

        while (applications && applications->data) {
            PanelTaskbarApplication *application
                = (PanelTaskbarApplication *)applications->data;

            if (!strcmp (application->id, self->m_id)) {
                self->m_application = application;

                panel_taskbar_application_add_toplevel (self->m_application,
                                                        self);

                return FALSE;
            }

            applications = applications->next;
        }

        self->m_application
            = panel_taskbar_application_new (self->m_id, self->m_taskbar);

        panel_taskbar_application_add_toplevel (self->m_application, self);
    }

    return FALSE;
}

static void
toplevel_handle_app_id (
    void *data, struct zwlr_foreign_toplevel_handle_v1 *toplevel_handle,
    const char *id) {
    UNUSED (toplevel_handle);

    PanelTaskbarToplevelButton *self = (PanelTaskbarToplevelButton *)data;

    self->m_id = g_strdup (id);

    self->m_icon_path = get_icon_from_app_id (self->m_taskbar, self->m_id);

    panel_taskbar_toplevel_button_rerender (self, false, true);

    g_idle_add ((GSourceFunc)toplevel_handle_app_id_gtk, self);
}

static gboolean
handle_closed_gtk (gpointer user_data) {
    PanelTaskbarToplevelButton *self = (PanelTaskbarToplevelButton *)user_data;

    panel_taskbar_application_remove_toplevel (self->m_application, self);
    self->m_application = NULL;

    if (self->m_title) {
        free (self->m_title);
    }

    if (self->m_id) {
        free (self->m_id);
    }

    if (self->m_icon_path) {
        free (self->m_icon_path);
    }

    free (self);

    return FALSE;
}

static void
toplevel_handle_closed (
    void *data, struct zwlr_foreign_toplevel_handle_v1 *toplevel_handle) {
    PanelTaskbarToplevelButton *self = (PanelTaskbarToplevelButton *)data;

    UNUSED (toplevel_handle);

    g_idle_add (G_SOURCE_FUNC (handle_closed_gtk), self);
}

void
toplevel_excess () {}

static gboolean
handle_state_activated_main_thread (PanelTaskbarToplevelButton *self) {
    self->m_application->toplevels
        = g_list_remove (self->m_application->toplevels, self);
    self->m_application->toplevels
        = g_list_prepend (self->m_application->toplevels, self);

    return FALSE;
}

static void
toplevel_handle_state (void *data,
                       struct zwlr_foreign_toplevel_handle_v1 *toplevel_handle,
                       struct wl_array *state) {
    UNUSED (toplevel_handle);

    PanelTaskbarToplevelButton *self = (PanelTaskbarToplevelButton *)data;

    uint32_t *entry;

    self->state = 0;

    wl_array_for_each (entry, state) {
        if (*entry == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MAXIMIZED) {
            self->state |= TOPLEVEL_STATE_MAXIMIZED;
        }
        if (*entry == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MINIMIZED) {
            self->state |= TOPLEVEL_STATE_MINIMIZED;
        }
        if (*entry == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED) {
            self->state |= TOPLEVEL_STATE_ACTIVATED;
        }
        if (*entry == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_FULLSCREEN) {
            self->state |= TOPLEVEL_STATE_FULLSCREEN;
        }
    }

    if (self->m_application && self->state & TOPLEVEL_STATE_ACTIVATED) {
        g_idle_add (G_SOURCE_FUNC (handle_state_activated_main_thread), self);
    }
}

static const struct zwlr_foreign_toplevel_handle_v1_listener toplevel_listener
    = {
          .title = toplevel_handle_title,
          .app_id = toplevel_handle_app_id,
          .closed = toplevel_handle_closed,
          .done = toplevel_excess,
          .output_enter = toplevel_excess,
          .output_leave = toplevel_excess,
          .parent = toplevel_excess,
          .state = toplevel_handle_state,
      };

static gboolean
button_click (GtkButton *button, PanelTaskbarToplevelButton *self) {
    UNUSED (button);

    if (self->state & TOPLEVEL_STATE_ACTIVATED) {
        zwlr_foreign_toplevel_handle_v1_set_minimized (
            self->m_toplevel_handle);
    } else if (self->state & TOPLEVEL_STATE_MINIMIZED) {
        zwlr_foreign_toplevel_handle_v1_unset_minimized (
            self->m_toplevel_handle);
    } else {
        zwlr_foreign_toplevel_handle_v1_activate (self->m_toplevel_handle,
                                                  self->m_seat);
    }

    wl_display_roundtrip (self->m_taskbar->display);

    return FALSE;
}

static gboolean
button_rerender_title_gtk (gpointer user_data) {
    PanelTaskbarToplevelButton *self = (PanelTaskbarToplevelButton *)user_data;

    if (GTK_IS_LABEL (self->label))
        gtk_label_set_text (GTK_LABEL (self->label), self->m_title);

    return FALSE;
}

static gboolean
button_rerender_app_id_and_icon_gtk (gpointer user_data) {
    PanelTaskbarToplevelButton *self = (PanelTaskbarToplevelButton *)user_data;

    if (GTK_IS_WIDGET (self->icon)) {
        gtk_box_remove (GTK_BOX (gtk_widget_get_parent (self->icon)),
                        self->icon);
    }
    if (GTK_IS_WIDGET (self->label)) {
        gtk_box_remove (GTK_BOX (gtk_widget_get_parent (self->label)),
                        self->label);
    }

    if (GTK_IS_WIDGET (self->box)) {
        gtk_box_remove (GTK_BOX (gtk_widget_get_parent (self->box)),
                        self->box);
    }

    GdkPixbuf *pbuf = NULL;
    if (self->m_icon_path)
        pbuf = gdk_pixbuf_new_from_file_at_size (self->m_icon_path, 48, 48,
                                                 NULL);

    if (pbuf) {
        self->icon = gtk_image_new_from_pixbuf (pbuf);

        g_object_unref (pbuf);
    } else {
        self->icon = gtk_image_new ();
    }

    gtk_widget_set_size_request (self->icon, 32, 32);

    self->label = gtk_label_new (self->m_title);

    gtk_widget_set_size_request (self->label, 60, 32);

    self->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_margin_end (GTK_WIDGET (self->box), 8);

    gtk_box_append (GTK_BOX (self->box), self->icon);
    gtk_box_append (GTK_BOX (self->box), self->label);

    gtk_button_set_child (GTK_BUTTON (self->rendered), self->box);

    return FALSE;
}

void
panel_taskbar_toplevel_button_rerender (PanelTaskbarToplevelButton *self,
                                        bool update_title,
                                        bool update_app_id_and_icon) {
    if (!self->rendered)
        return;

    if (update_title) {
        g_idle_add ((GSourceFunc)button_rerender_title_gtk, self);
    }

    if (update_app_id_and_icon) {
        g_idle_add ((GSourceFunc)button_rerender_app_id_and_icon_gtk, self);
    }
}

void
panel_taskbar_toplevel_button_gtk_run (PanelTaskbarToplevelButton *self) {
    if (self->rendered)
        return;

    self->rendered = gtk_button_new ();
    gtk_widget_set_name (self->rendered, "panel_popover_button");
    g_signal_connect (self->rendered, "clicked", G_CALLBACK (button_click),
                      self);

    button_rerender_app_id_and_icon_gtk (self);
}

PanelTaskbarToplevelButton *
panel_taskbar_toplevel_button_new (
    struct zwlr_foreign_toplevel_handle_v1 *toplevel_handle,
    struct wl_seat *seat, PanelTaskbar *taskbar) {
    PanelTaskbarToplevelButton *self
        = malloc (sizeof (PanelTaskbarToplevelButton));

    self->rendered = NULL;

    self->box = NULL;

    self->icon = NULL;
    self->label = NULL;

    self->m_toplevel_handle = toplevel_handle;
    self->m_seat = seat;
    self->m_maximized = self->m_activated = self->m_minimized
        = self->m_fullscreen = false;
    self->m_taskbar = taskbar;
    self->m_application = NULL;

    self->m_title = self->m_id = self->m_icon_path = NULL;
    self->m_output = NULL;
    self->m_state = NULL;

    zwlr_foreign_toplevel_handle_v1_add_listener (self->m_toplevel_handle,
                                                  &toplevel_listener, self);

    return self;
}