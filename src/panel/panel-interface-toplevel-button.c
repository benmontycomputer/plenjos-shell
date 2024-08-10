#include "panel-interface-toplevel-button.h"

typedef struct {
    char *exec;
    char *icon;
} icon_exec_map_item;

void
free_icon_exec_map_item (icon_exec_map_item *item) {
    if (item) {
        if (item->exec)
            free (item->exec);
        if (item->icon)
            free (item->icon);

        free (item);
    }
}

static GList *init_icon_exec_map ();

static int
icon_exec_map_finder (gpointer item, gchar *exec) {
    if (item && exec && !strcmp (((icon_exec_map_item *)item)->exec, exec)) {
        return 0;
    }

    return 1;
}

static void
toplevel_handle_title (void *data,
                       struct zwlr_foreign_toplevel_handle_v1 *toplevel_handle,
                       const char *title) {
    PanelInterfaceToplevelButton *self = (PanelInterfaceToplevelButton *)data;

    UNUSED (toplevel_handle);

    self->m_title = g_strdup (title);
    // printf ("%s\n", title);
    // fflush (stdout);

    panel_interface_toplevel_button_rerender (self, true, false);
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

void
toplevel_update_icon_from_app_id (void *data, char *id) {
    PanelInterfaceToplevelButton *self = (PanelInterfaceToplevelButton *)data;

    char *icon = NULL;

    // Is this icon already loaded?
    // TODO: actually check if icon is already loaded

    if (icon == NULL) {
        icon = suggested_icon_for_id (id, 48);
    }
    if (icon == NULL) {
        char *mod_id = g_strdup (id);

        remove_spaces (mod_id);
        icon = suggested_icon_for_id (mod_id, 48);

        free (mod_id);
    }
    if (icon == NULL) {
        char *mod_id = g_strdup (id);

        for (size_t i = 0; mod_id[i]; i++) {
            mod_id[i] = tolower (mod_id[i]);
        }
        icon = suggested_icon_for_id (mod_id, 48);

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

        icon = suggested_icon_for_id (mod_id, 48);

        if (icon == NULL) {
            for (size_t i = 0; mod_id[i]; i++) {
                mod_id[i] = tolower (mod_id[i]);
            }

            icon = suggested_icon_for_id (mod_id, 48);
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

        icon = suggested_icon_for_id ((mod_id + pos), 48);

        if (icon == NULL) {
            for (size_t i = 0; mod_id[i]; i++) {
                mod_id[i] = tolower (mod_id[i]);
            }

            icon = suggested_icon_for_id ((mod_id + pos), 48);
        }

        free (mod_id);
    }
    if (icon == NULL) {
        GList *icon_exec_map = init_icon_exec_map ();

        GList *found = g_list_find_custom (icon_exec_map, id,
                                           (GCompareFunc)icon_exec_map_finder);

        if (found != NULL) {
            icon = suggested_icon_for_id (
                ((icon_exec_map_item *)found->data)->icon, 48);
            g_list_free_full (found, (GDestroyNotify)free_icon_exec_map_item);
        }

        g_list_free_full (icon_exec_map,
                          (GDestroyNotify)free_icon_exec_map_item);
    }
    if (icon == NULL) {
        icon = suggested_icon_for_id ("emblem-dialog-question", 48);
    }

    printf ("Id: %s, icon: %s\n", self->m_id, icon);
    fflush (stdout);

    self->m_icon_path = icon;
}

static void
toplevel_handle_app_id (
    void *data, struct zwlr_foreign_toplevel_handle_v1 *toplevel_handle,
    const char *id) {
    UNUSED (toplevel_handle);

    PanelInterfaceToplevelButton *self = (PanelInterfaceToplevelButton *)data;

    self->m_id = g_strdup (id);

    toplevel_update_icon_from_app_id (self, self->m_id);

    panel_interface_toplevel_button_rerender (self, false, true);
}

gboolean
handle_closed_gtk (gpointer user_data) {
    PanelInterfaceToplevelButton *self
        = (PanelInterfaceToplevelButton *)user_data;

    gtk_container_remove (
        GTK_CONTAINER (gtk_widget_get_parent (self->m_rendered)),
        self->m_rendered);

    return FALSE;
}

static void
toplevel_handle_closed (
    void *data, struct zwlr_foreign_toplevel_handle_v1 *toplevel_handle) {
    PanelInterfaceToplevelButton *self = (PanelInterfaceToplevelButton *)data;

    UNUSED (toplevel_handle);

    gdk_threads_add_idle ((GSourceFunc)handle_closed_gtk, self);
}

void
toplevel_excess () {}

static void
toplevel_handle_state (
    void *data,
    struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1,
    struct wl_array *state) {
    printf("%d\n", ((enum zwlr_foreign_toplevel_handle_v1_state)state[0].data) && ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MINIMIZED);
    fflush(stdout);
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

gboolean
button_click (GtkButton *button, PanelInterfaceToplevelButton *self){
    
}

gboolean button_rerender_app_id_and_icon_gtk (gpointer user_data) {
    PanelInterfaceToplevelButton *self
        = (PanelInterfaceToplevelButton *)user_data;

    if (self->icon) {
        gtk_container_remove (
            GTK_CONTAINER (gtk_widget_get_parent (self->icon)), self->icon);
    }

    if (self->button) {
        gtk_container_remove (
            GTK_CONTAINER (gtk_widget_get_parent (self->button)),
            self->button);
    }

    self->icon = gtk_image_new_from_pixbuf (
        gdk_pixbuf_new_from_file_at_size (self->m_icon_path, 48, 48, NULL));

    self->button = gtk_button_new ();
    gtk_widget_set_name (self->button, "taskbar_button");

    gtk_button_set_image (GTK_BUTTON (self->button), self->icon);
    g_signal_connect (self->button, "clicked", (GCallback)button_click, self);

    gtk_container_add (GTK_CONTAINER (self->m_rendered), self->button);

    gtk_widget_show_all (self->m_rendered);

    return FALSE;
}

void
panel_interface_toplevel_button_rerender (PanelInterfaceToplevelButton *self,
                                          bool update_title,
                                          bool update_app_id_and_icon) {
    if (update_title) {
    }

    if (update_app_id_and_icon) {
        gdk_threads_add_idle ((GSourceFunc)button_rerender_app_id_and_icon_gtk,
                              self);
    }
}

PanelInterfaceToplevelButton *
panel_interface_toplevel_button_new (
    struct zwlr_foreign_toplevel_handle_v1 *toplevel_handle,
    struct wl_seat *seat, PanelInterface *interface) {
    PanelInterfaceToplevelButton *self
        = malloc (sizeof (PanelInterfaceToplevelButton));

    // TODO: check if gtk_box_new needs to be in gdk_threads_add_idle
    self->m_rendered = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    self->icon = NULL;
    self->button = NULL;

    self->m_toplevel_handle = toplevel_handle;
    self->m_seat = seat;
    self->m_maximized = self->m_activated = self->m_minimized
        = self->m_fullscreen = false;
    self->m_interface = interface;

    zwlr_foreign_toplevel_handle_v1_add_listener (self->m_toplevel_handle,
                                                  &toplevel_listener, self);

    return self;
}

GList *
init_icon_exec_map () {
    GList *return_val = g_list_alloc ();

    char *xdg_data_dirs = g_strdup (getenv ("XDG_DATA_DIRS"));

    str_split_return_val str_split_result = str_split (xdg_data_dirs, ':');

    str_split_result.result
        = realloc (str_split_result.result,
                   (str_split_result.count + 2) * sizeof (char *));

    // Don't free.
    const char *home_path = getenv ("HOME");

    size_t home_apps_dir_len
        = strlen (home_path) + strlen ("/.local/share/applications/") + 1;
    str_split_result.result[str_split_result.count]
        = malloc (home_apps_dir_len);

    snprintf (str_split_result.result[str_split_result.count],
              home_apps_dir_len, "%s/.local/share/applications/", home_path);
    str_split_result.result[str_split_result.count + 1] = NULL;

    str_split_result.count++;

    for (size_t i = 0; i < str_split_result.count; i++) {
        char *dir = str_split_result.result[i];

        // https://stackoverflow.com/questions/1271064/how-do-i-loop-through-all-files-in-a-folder-using-c
        struct dirent *dp;
        DIR *dfd;

        if ((dfd = opendir (dir)) != NULL) {
            while ((dp = readdir (dfd)) != NULL) {
                if (dp->d_name[0] != '.') {
                    char *fname_split = g_strdup (dp->d_name);
                    str_split_return_val fname_split_result
                        = str_split (fname_split, '.');

                    if (!strcmp (fname_split_result
                                     .result[fname_split_result.count - 1],
                                 ".desktop")) {
                        size_t full_path_len
                            = strlen (dir) + strlen (dp->d_name) + 2;
                        char *full_path = malloc (full_path_len);
                        snprintf (full_path, full_path_len, "%s/%s", dir,
                                  dp->d_name);

                        GKeyFile *kf = g_key_file_new ();
                        g_key_file_load_from_file (kf, full_path,
                                                   G_KEY_FILE_NONE, NULL);

                        icon_exec_map_item *item
                            = malloc (sizeof (icon_exec_map_item));

                        item->exec = g_key_file_get_string (
                            kf, G_KEY_FILE_DESKTOP_GROUP,
                            G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);
                        item->icon = g_key_file_get_string (
                            kf, G_KEY_FILE_DESKTOP_GROUP,
                            G_KEY_FILE_DESKTOP_KEY_ICON, NULL);

                        if (item->exec && item->icon)
                            return_val = g_list_append (return_val, item);
                    }

                    free_string_list (fname_split_result.result);
                    free (fname_split);
                }
            }
        }
    }

    return return_val;
}