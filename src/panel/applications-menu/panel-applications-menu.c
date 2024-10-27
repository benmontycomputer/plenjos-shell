/* panel-applications-menu.c
 *
 * Copyright 2023 Benjamin Montgomery
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "panel-applications-menu.h"

#define MARKUP_FORMAT "<span size=\"small\">%s</span>"

struct _PanelApplicationsMenu {
    GtkApplicationWindow parent_instance;

    GtkScrolledWindow *scrolled_window;

    GtkFlowBox *apps_flow_box;
    // GtkGrid *favorites_grid;

    gboolean supports_alpha;

    size_t items_count;
    size_t favorites_count;

    GtkIconTheme *icon_theme;
    int icon_size;
    int scale;

    GdkPixbuf *desktop_blurred;

    GSettings *settings;

    GtkWidget *launcher_button;
};

G_DEFINE_TYPE (PanelApplicationsMenu, panel_applications_menu,
               GTK_TYPE_APPLICATION_WINDOW)

static void
panel_applications_menu_class_init (PanelApplicationsMenuClass *klass) {
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (
        widget_class,
        "/com/plenjos/Panel/applications-menu/panel-applications-menu.ui");
    gtk_widget_class_bind_template_child (widget_class, PanelApplicationsMenu,
                                          apps_flow_box);
    // gtk_widget_class_bind_template_child (widget_class,
    // PanelApplicationsMenu, favorites_grid);
    gtk_widget_class_bind_template_child (widget_class, PanelApplicationsMenu,
                                          scrolled_window);
}

void
hide_applications_menu (PanelApplicationsMenu *self) {
    gtk_widget_hide (GTK_WIDGET (&self->parent_instance));
}

void
show_applications_menu (PanelApplicationsMenu *self) {
    gtk_widget_show (GTK_WIDGET (&self->parent_instance));
}

static gboolean
check_escape (GtkEventControllerKey *key_controller, guint keyval,
              guint keycode, GdkModifierType state,
              PanelApplicationsMenu *self) {
    UNUSED (key_controller);
    UNUSED (keycode);
    UNUSED (state);

    if (keyval == GDK_KEY_Escape) {
        hide_applications_menu (self);
        return TRUE;
    }
    return FALSE;
}

static void
focus_out_event (GtkEventControllerFocus *focus_controller,
                 PanelApplicationsMenu *self) {
    UNUSED (focus_controller);

    hide_applications_menu (self);
}

typedef struct {
    char *exec;
    PanelApplicationsMenu *self;
} ClickedArgs;

static void
applications_menu_app_clicked (GtkButton *widget, ClickedArgs *args) {
    UNUSED (widget);

    hide_applications_menu (args->self);
    size_t new_exec_len = strlen (args->exec) + 3;
    char *new_exec = malloc (new_exec_len);
    snprintf (new_exec, new_exec_len, "%s &", args->exec);
    system (new_exec);
    free (new_exec);
}

static GtkWidget *
applications_menu_render_app (PanelApplicationsMenu *self, char *icon_name,
                              char *exec_path, char *display_name) {
    GtkButton *button = GTK_BUTTON (gtk_button_new ());
    gtk_widget_set_name (GTK_WIDGET (button), "panel_applications_menu_item");
    GtkBox *box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 2));

    GtkImage *icon = GTK_IMAGE (gtk_image_new ());

    bool fallback = true;

    if (icon_name) {
        if (access (icon_name, F_OK) == 0) {
            GdkPixbuf *icon_pbuf = gdk_pixbuf_new_from_file_at_size (
                icon_name, self->icon_size, self->icon_size, NULL);

            if (icon) {
                gtk_image_set_from_pixbuf (icon, icon_pbuf);

                fallback = false;

                g_object_unref (icon_pbuf);
            }
        }

        if (fallback) {
            if (gtk_icon_theme_has_icon (self->icon_theme, icon_name)) {
                GtkIconPaintable *icon_paintable = gtk_icon_theme_lookup_icon (
                    self->icon_theme, icon_name, NULL, self->icon_size,
                    self->scale, GTK_TEXT_DIR_LTR, 0);

                GFile *file = gtk_icon_paintable_get_file (icon_paintable);

                gchar *filename = g_file_get_path (file);

                GdkPixbuf *icon_pbuf = gdk_pixbuf_new_from_file_at_size (
                    filename, self->icon_size, self->icon_size, NULL);

                if (icon) {
                    gtk_image_set_from_pixbuf (icon, icon_pbuf);

                    fallback = false;

                    g_object_unref (icon_pbuf);
                }

                free (filename);

                g_object_unref (file);

                fallback = false;

                g_object_unref (icon_paintable);
            }
        }
    }
    if (fallback) {
        // Don't need to check if the icon theme contains "dialog-question"
        // because gtk_icon_theme_lookup_icon will return a placeholder if
        // it doesn't exist.

        GtkIconPaintable *icon_paintable = gtk_icon_theme_lookup_icon (
            self->icon_theme, "dialog-question", NULL, self->icon_size,
            self->scale, GTK_TEXT_DIR_LTR, 0);

        GFile *file = gtk_icon_paintable_get_file (icon_paintable);

        gchar *filename = g_file_get_path (file);

        GdkPixbuf *icon_pbuf = gdk_pixbuf_new_from_file_at_size (
            filename, self->icon_size, self->icon_size, NULL);

        if (icon) {
            gtk_image_set_from_pixbuf (icon, icon_pbuf);

            fallback = false;

            g_object_unref (icon_pbuf);
        }

        free (filename);

        g_object_unref (file);

        fallback = false;

        g_object_unref (icon_paintable);
    }

    gtk_widget_set_size_request (GTK_WIDGET (icon), self->icon_size,
                                 self->icon_size);

    GtkLabel *label = GTK_LABEL (gtk_label_new (""));
    gtk_widget_set_size_request (GTK_WIDGET (label), self->icon_size + 24, 30);
    gtk_label_set_wrap (label, TRUE);
    gtk_label_set_wrap_mode (label, PANGO_WRAP_WORD_CHAR);
    gtk_label_set_max_width_chars (label, 0);
    gtk_label_set_justify (label, GTK_JUSTIFY_CENTER);

    size_t len = 30;
    char *name = malloc (len);
    snprintf (name, len, "%s", display_name);

    if (display_name && strlen (display_name) >= len) {
        name[len - 4] = '.';
        name[len - 3] = '.';
        name[len - 2] = '.';
        gtk_widget_set_tooltip_text (GTK_WIDGET (label), display_name);
    }

    // gtk_label_set_text (label, name);

    char *markup = NULL;

    markup = g_markup_printf_escaped (MARKUP_FORMAT, name);

    free (name);

    gtk_label_set_markup (label, markup);

    free (markup);

    gtk_box_append (box, GTK_WIDGET (icon));
    gtk_box_append (box, GTK_WIDGET (label));
    gtk_button_set_child (button, GTK_WIDGET (box));

    ClickedArgs *clicked_args = malloc (sizeof (ClickedArgs));

    clicked_args->exec = g_strdup (exec_path);
    clicked_args->self = self;

    g_signal_connect (button, "clicked",
                      G_CALLBACK (applications_menu_app_clicked),
                      clicked_args);

    return GTK_WIDGET (button);
}

void
panel_applications_menu_set_monitor (PanelApplicationsMenu *self,
                                     GdkMonitor *monitor) {
    GdkRectangle geo;
    gdk_monitor_get_geometry (monitor, &geo);

    gtk_widget_set_margin_start (GTK_WIDGET (self->scrolled_window),
                                 geo.width / 6);
    gtk_widget_set_margin_end (GTK_WIDGET (self->scrolled_window),
                               geo.width / 6);
}

static void
applications_menu_add_app_from (PanelApplicationsMenu *self, char *icon_name,
                                char *exec_path, char *display_name) {
    GtkWidget *button = applications_menu_render_app (self, icon_name,
                                                      exec_path, display_name);

    gtk_flow_box_append (self->apps_flow_box, GTK_WIDGET (button));

    self->items_count++;
}

void
panel_applications_menu_set_bg (PanelApplicationsMenu *self, GdkPixbuf *bg) {
    self->desktop_blurred = bg;
}

static int
sort_apps (GKeyFile *kf1, GKeyFile *kf2) {
    gchar *name1 = g_key_file_get_string (kf1, G_KEY_FILE_DESKTOP_GROUP,
                                          G_KEY_FILE_DESKTOP_KEY_NAME, NULL);
    gchar *name2 = g_key_file_get_string (kf2, G_KEY_FILE_DESKTOP_GROUP,
                                          G_KEY_FILE_DESKTOP_KEY_NAME, NULL);

    if (!name1) {
        if (name2)
            free (name2);
        return 1;
    }
    if (!name2) {
        // We don't need to check if name1 is null because the function would
        // have already exited if it was.
        free (name1);
        return -1;
    }
    // https://stackoverflow.com/questions/2661766/how-do-i-lowercase-a-string-in-c
    for (size_t i = 0; name1[i]; i++) {
        name1[i] = tolower (name1[i]);
    }
    for (size_t i = 0; name2[i]; i++) {
        name2[i] = tolower (name2[i]);
    }
    int returnval = strcmp (name1, name2);
    free (name1);
    free (name2);
    return returnval;
}

// https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
static char **
str_split_dashboard (char *a_str, const char a_delim) {
    char **result = 0;
    size_t count = 0;
    char *tmp = a_str;
    char *last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen (a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc (sizeof (char *) * count);

    if (result) {
        size_t idx = 0;
        char *token = strtok (a_str, delim);

        while (token) {
            assert (idx < count);
            *(result + idx++) = strdup (token);
            token = strtok (0, delim);
        }
        assert (idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

static void
panel_applications_menu_init (PanelApplicationsMenu *self) {
    gtk_widget_init_template (GTK_WIDGET (self));

    // Before the window is first realized, set it up to be a layer surface
    gtk_layer_init_for_window (GTK_WINDOW (&self->parent_instance));

    // Order below normal windows
    gtk_layer_set_layer (GTK_WINDOW (&self->parent_instance),
                         GTK_LAYER_SHELL_LAYER_OVERLAY);

    gtk_layer_set_keyboard_mode (
        GTK_WINDOW (&self->parent_instance),
        GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE); // NONE is default

    // Push other windows out of the way
    gtk_layer_set_exclusive_zone (GTK_WINDOW (&self->parent_instance), -1);

    // We don't need to get keyboard input
    gtk_layer_set_keyboard_mode (
        GTK_WINDOW (&self->parent_instance),
        GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE); // NONE is default

    // Anchors are if the window is pinned to each edge of the output
    static const gboolean anchors[] = { TRUE, TRUE, TRUE, TRUE };
    for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++) {
        gtk_layer_set_anchor (GTK_WINDOW (&self->parent_instance),
                              (GtkLayerShellEdge)i, anchors[i]);
    }

    GtkEventController *key_controller = gtk_event_controller_key_new ();
    GtkEventController *focus_controller = gtk_event_controller_focus_new ();

    gtk_widget_add_controller (GTK_WIDGET (&self->parent_instance),
                               key_controller);
    gtk_widget_add_controller (GTK_WIDGET (&self->parent_instance),
                               focus_controller);

    g_signal_connect (key_controller, "key-pressed", G_CALLBACK (check_escape),
                      self);
    g_signal_connect (focus_controller, "leave", G_CALLBACK (focus_out_event),
                      self);

    gtk_widget_show (GTK_WIDGET (&self->parent_instance));

    self->icon_theme = gtk_icon_theme_get_for_display (
        gtk_widget_get_display (GTK_WIDGET (&self->parent_instance)));
    self->icon_size = 96;
    self->scale
        = gtk_widget_get_scale_factor (GTK_WIDGET (&self->parent_instance));

    char *data_dirs = g_strdup (getenv ("XDG_DATA_DIRS"));

    if (!data_dirs) {
        size_t len = strlen (DATA_DIRS_DEFAULT) + 1;
        data_dirs = malloc (len);
        snprintf (data_dirs, len, "%s", DATA_DIRS_DEFAULT);
    }

    GList *applications = NULL;

    if (!data_dirs) {
        fprintf (stderr, "Error: XDG_DATA_DIRS is not set. Apps menu may not "
                         "load applications.\n");
        fflush (stderr);
    } else {
        // https://stackoverflow.com/questions/7704144/how-do-i-use-glib-or-any-other-library-to-list-all-the-files-in-a-directory
        char **data_dirs_list = str_split_dashboard (data_dirs, ':');

        for (size_t data_dir_index = 0; data_dirs_list[data_dir_index];
             data_dir_index++) {
            size_t applications_dir_len
                = strlen (data_dirs_list[data_dir_index])
                  + strlen ("/applications/") + 1;
            char *applications_dir = malloc (applications_dir_len);

            snprintf (applications_dir, applications_dir_len,
                      "%s/applications/", data_dirs_list[data_dir_index]);

            DIR *d;
            struct dirent *dir;

            d = opendir (applications_dir);
            if (d) {
                while ((dir = readdir (d)) != NULL) {
                    GKeyFile *key_file = g_key_file_new ();

                    size_t len
                        = strlen (applications_dir) + strlen (dir->d_name) + 1;
                    char *path = malloc (len);

                    snprintf (path, len, "%s%s", applications_dir,
                              dir->d_name);

                    // printf("%s\n", path);
                    // fflush(stdout);
                    if (g_key_file_load_from_file (key_file, path,
                                                   G_KEY_FILE_NONE, NULL)) {
                        // printf("%s\n", path);
                        applications = g_list_append (applications, key_file);
                    } else {
                        g_key_file_free (key_file);
                    }

                    free (path);
                }
                closedir (d);
            }

            free (applications_dir);
        }

        fflush (stdout);

        applications = g_list_sort (applications, (GCompareFunc)sort_apps);

        for (GList *application = applications; application;
             application = application->next) {
            gchar *name = g_key_file_get_string (
                (GKeyFile *)application->data, G_KEY_FILE_DESKTOP_GROUP,
                G_KEY_FILE_DESKTOP_KEY_NAME, NULL);
            gchar *exec = g_key_file_get_string (
                (GKeyFile *)application->data, G_KEY_FILE_DESKTOP_GROUP,
                G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);
            gchar *icon = g_key_file_get_string (
                (GKeyFile *)application->data, G_KEY_FILE_DESKTOP_GROUP,
                G_KEY_FILE_DESKTOP_KEY_ICON, NULL);

            if (name && exec) {
                applications_menu_add_app_from (self, icon, exec, name);
            }

            free (name);
            free (exec);
            free (icon);

            g_key_file_free ((GKeyFile *)application->data);
        }

        g_list_free (applications);

        free_string_list (data_dirs_list);
        free (data_dirs);
    }

    GdkRectangle geo;
    gdk_monitor_get_geometry (
        gdk_display_get_monitor_at_surface (
            gdk_display_get_default (),
            gtk_native_get_surface (GTK_NATIVE (&self->parent_instance))),
        &geo);

    // gtk_widget_set_size_request (GTK_WIDGET (&self->parent_instance),
    //                              geo.width, geo.height);

    gtk_widget_set_margin_start (GTK_WIDGET (self->scrolled_window),
                                 geo.width / 6);
    gtk_widget_set_margin_end (GTK_WIDGET (self->scrolled_window),
                               geo.width / 6);
}

static void
show_wrap (GtkButton *button, PanelApplicationsMenu *self) {
    UNUSED (button);

    show_applications_menu (self);
}

GtkWidget *
panel_applications_menu_insert_launcher_button (PanelApplicationsMenu *self,
                                                GtkBox *box) {
    if (!self->launcher_button) {
        self->launcher_button = gtk_button_new ();

        gtk_widget_set_name (self->launcher_button, "panel_button");

        gtk_widget_set_valign (self->launcher_button, GTK_ALIGN_START);

        GtkWidget *img = gtk_image_new_from_icon_name ("view-app-grid");

        gtk_widget_set_size_request (img, 48, 48);
        gtk_button_set_child (GTK_BUTTON (self->launcher_button), img);

        g_signal_connect (self->launcher_button, "clicked",
                          G_CALLBACK (show_wrap), self);
    }

    gtk_box_prepend (box, self->launcher_button);

    return self->launcher_button;
}