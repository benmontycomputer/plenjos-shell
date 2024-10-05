#include "hyprland/workspaces.h"
#include "tray/panel-tray.h"

char *
strdup (const char *src) {
    if (src == NULL) {
        return NULL;
    }

    size_t len = strlen (src) + 1; // String plus '\0'
    char *dst = malloc (len);      // Allocate space
    if (dst == NULL)
        return NULL;        // No memory
    memcpy (dst, src, len); // Copy the block
    return dst;             // Return the new string
}

static gboolean
removeWorkspaceGtk (GtkWidget *widget) {
    GtkWidget *parent = gtk_widget_get_parent (widget);

    if (GTK_IS_BOX (parent)) {
        gtk_box_remove (GTK_BOX (parent), widget);
    }

    return FALSE;
}

void
removeWorkspace (int workspace_id, HyprBackend *self) {
    if (workspace_id >= WORKSPACES_MAX) {
        fprintf (stderr,
                 "Trying to remove a workspace >=WORKSPACES_MAX. Abort.\n");
        fflush (stderr);

        return;
    }

    if (workspace_id < 0) {
        fprintf (stderr, "Trying to remove a workspace <0. Abort.\n");
        fflush (stderr);

        return;
    }

    HyprWorkspace *workspace_ptr = self->workspaces[workspace_id];

    if (!workspace_ptr) {
        fprintf (stderr,
                 "Trying to remove workspace %d but it doesn't exist.\n",
                 workspace_id);
        fflush (stderr);

        return;
    }

    // TODO: update tray

    g_idle_add ((GSourceFunc)removeWorkspaceGtk, workspace_ptr->button);

    if (workspace_ptr->name)
        free (workspace_ptr->name);
    if (workspace_ptr->monitor)
        free (workspace_ptr->monitor);
    if (workspace_ptr->lastwindow)
        free (workspace_ptr->lastwindow);
    if (workspace_ptr->lastwindowtitle)
        free (workspace_ptr->lastwindowtitle);

    free (workspace_ptr);

    self->workspaces[workspace_id] = NULL;
}

typedef struct {
    int workspace_id;
    HyprBackend *backend;
    bool hold;
} WorkspaceArgs;

#define SWITCH_WORKSPACE "dispatch workspace "

static void switch_workspace (GtkButton *button, WorkspaceArgs *args) {
    size_t rq_len = strlen (SWITCH_WORKSPACE) + 16;
    char *rq = malloc (rq_len);

    snprintf (rq, rq_len, "%s%d", SWITCH_WORKSPACE, args->workspace_id);

    free (getSocket1Reply (rq));
}

static void free_args (GtkWidget *widget, WorkspaceArgs *args) {
    free (args);
}

static gboolean
addWorkspaceGtk (WorkspaceArgs *args) {
    HyprWorkspace *workspace_ptr = args->backend->workspaces[args->workspace_id];
    HyprBackend *self = args->backend;

    if (!workspace_ptr->gdk_monitor) {
        GListModel *monitors = self->panel->monitors;

        size_t n_monitors = g_list_model_get_n_items (monitors);

        for (size_t i = 0; i < n_monitors; i++) {
            gpointer monitor_ptr = g_list_model_get_item (monitors, i);
            if (GDK_IS_MONITOR (monitor_ptr)) {
                GdkMonitor *monitor = GDK_MONITOR (monitor_ptr);

                const char *output = gdk_monitor_get_connector (monitor);

                if (!strcmp (output, workspace_ptr->monitor)) {
                    workspace_ptr->gdk_monitor = monitor;
                    printf ("Workspace: %d %s. Output: %s.\n",
                            workspace_ptr->id, workspace_ptr->name, output);
                    break;
                }
            }
        }
    }

    GtkWidget *button = gtk_button_new_with_label (workspace_ptr->name);

    gtk_widget_set_name (button, "panel_tray_workspace_button");

    WorkspaceArgs *new_args = malloc (sizeof (WorkspaceArgs));

    new_args->backend = self;
    new_args->workspace_id = args->workspace_id;
    new_args->hold = false;

    g_signal_connect (button, "clicked", G_CALLBACK (switch_workspace), new_args);
    g_signal_connect (button, "destroy", G_CALLBACK (free_args), new_args);

    workspace_ptr->button = button;

    PanelTray *tray = (PanelTray *)self->panel->tray;

    for (size_t i = 0; tray->windows[i]; i++) {
        PanelTrayWindow *tray_win = tray->windows[i];

        if (tray_win->monitor == workspace_ptr->gdk_monitor) {
            gtk_box_append (tray_win->workspaces_box, button);

            break;
        }
    }

    args->hold = false;

    return FALSE;
}

// This function tries to figure out the GdkMonitor automatically.
void
addWorkspace (HyprWorkspace workspace, HyprBackend *self, bool override) {
    int workspace_id = workspace.id;

    if (workspace_id >= WORKSPACES_MAX) {
        fprintf (stderr,
                 "Trying to add a workspace >=WORKSPACES_MAX. Abort.\n");
        fflush (stderr);

        return;
    }

    if (workspace_id < 0) {
        fprintf (stderr, "Trying to add a workspace <0. Abort.\n");
        fflush (stderr);

        return;
    }

    if (self->workspaces[workspace_id]) {
        if (override) {
            removeWorkspace (workspace_id, self);
        } else {
            fprintf (stderr,
                     "Trying to add workspace with id %d but that workspace "
                     "already exists.\n",
                     workspace_id);
            fflush (stderr);

            return;
        }
    }

    HyprWorkspace *workspace_ptr = malloc (sizeof (HyprWorkspace));
    workspace_ptr[0] = workspace;

    self->workspaces[workspace_id] = workspace_ptr;

    WorkspaceArgs *args = malloc (sizeof (WorkspaceArgs));

    args->workspace_id = workspace_id;
    args->backend = self;
    args->hold = true;

    g_idle_add ((GSourceFunc)addWorkspaceGtk, (gpointer)args);

    while (args->hold) {
        usleep (1);
    }

    free (args);
}

void
updateWorkspaces (HyprBackend *self) {
    char *workspaces_init = getSocket1Reply ("j/workspaces");

    // https://jansson.readthedocs.io/en/latest/tutorial.html
    json_t *root;
    json_error_t error;

    root = json_loads (workspaces_init, 0, &error);
    free (workspaces_init);

    if (!root) {
        fprintf (
            stderr,
            "Couldn't parse initial workspaces JSON. error on line %d: %s\n",
            error.line, error.text);
        fflush (stderr);
    } else {
        if (!json_is_array (root)) {
            fprintf (stderr, "Error: root node for initial workspaces JSON is "
                             "not an array.\n");
            fflush (stderr);
        } else {
            for (size_t i = 0; i < WORKSPACES_MAX; i++) {
                if (self->workspaces[i]) {
                    removeWorkspace (i, self);
                }
            }

            size_t workspacesCount = json_array_size (root);

            for (size_t i = 0; i < workspacesCount; i++) {
                json_t *data, *id, *name, *monitor, *monitorID, *windows,
                    *hasfullscreen, *lastwindow, *lastwindowtitle;

                data = json_array_get (root, i);
                if (!json_is_object (data)) {
                    fprintf (stderr,
                             "error: workspace data at index %ld is not an "
                             "object\n",
                             i);
                    fflush (stderr);
                } else {
                    // TODO: check types on these

                    id = json_object_get (data, "id");
                    name = json_object_get (data, "name");

                    int id_int = json_integer_value (id);
                    const char *name_str = json_string_value (name);

                    monitor = json_object_get (data, "monitor");
                    monitorID = json_object_get (data, "monitorID");
                    windows = json_object_get (data, "windows");
                    hasfullscreen = json_object_get (data, "hasfullscreen");
                    lastwindow = json_object_get (data, "lastwindow");
                    lastwindowtitle
                        = json_object_get (data, "lastwindowtitle");

                    HyprWorkspace workspace = {
                        id : id_int,
                        name : strdup (name_str),
                        monitor : strdup (json_string_value (monitor)),
                        monitorID : json_integer_value (monitorID),
                        windows : json_integer_value (windows),
                        hasfullscreen : json_boolean_value (hasfullscreen),
                        lastwindow : strdup (json_string_value (lastwindow)),
                        lastwindowtitle :
                            strdup (json_string_value (lastwindowtitle)),
                        gdk_monitor : NULL,
                        button : NULL,
                    };

                    // TODO: stop this from potentially breaking the
                    // program
                    if (id_int >= WORKSPACES_MAX || id_int < 0) {
                        fprintf (stderr,
                                 "workspace id %d is greater than the max of "
                                 "%d. Can't continue.\n",
                                 id_int, WORKSPACES_MAX);
                        fflush (stderr);

                        continue;
                    }

                    addWorkspace (workspace, self, true);
                }
            }
        }

        json_decref (root);
    }
}

void
onWorkspaceActivated (char *payload, HyprBackend *self) {}

void
onSpecialWorkspaceActivated (char *payload, HyprBackend *self) {}

void
onWorkspaceDestroyed (char *payload, HyprBackend *self) {
    updateWorkspaces (self);
}

void
onWorkspaceCreated (char *payload, HyprBackend *self) {
    updateWorkspaces (self);
}

void
onMonitorFocused (char *payload, HyprBackend *self) {}

void
onWorkspaceMoved (char *payload, HyprBackend *self) {
    updateWorkspaces (self);
}

void
onWindowOpened (char *payload, HyprBackend *self) {}

void
onWindowClosed (char *payload, HyprBackend *self) {}

void
onWindowMoved (char *payload, HyprBackend *self) {}

void
setUrgentWorkspace (char *payload, HyprBackend *self) {}

void
onWorkspaceRenamed (char *payload, HyprBackend *self) {
    updateWorkspaces (self);
}

void
onWindowTitleEvent (char *payload, HyprBackend *self) {}

void
onConfigReloaded (HyprBackend *self) {}