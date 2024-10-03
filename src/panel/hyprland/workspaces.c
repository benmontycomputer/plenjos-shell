#include "hyprland/workspaces.h"

char *
strdup (const char *src) {
    size_t len = strlen (src) + 1; // String plus '\0'
    char *dst = malloc (len);      // Allocate space
    if (dst == NULL)
        return NULL;        // No memory
    memcpy (dst, src, len); // Copy the block
    return dst;             // Return the new string
}

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

    // TODO: update trays
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

void
updateWorkspace (int workspace_id, HyprBackend *self) {
    char *workspaces_init = getSocket1Reply ("j/workspaces");

    printf ("\n%s\n", workspaces_init);
    fflush (stdout);

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

                    int id_int = json_integer_value (id);

                    if ((workspace_id == UPDATE_ALL_WORKSPACES)
                        || id_int == workspace_id) {
                        name = json_object_get (data, "name");
                        monitor = json_object_get (data, "monitor");
                        monitorID = json_object_get (data, "monitorID");
                        windows = json_object_get (data, "windows");
                        hasfullscreen
                            = json_object_get (data, "hasfullscreen");
                        lastwindow = json_object_get (data, "lastwindow");
                        lastwindowtitle
                            = json_object_get (data, "lastwindowtitle");

                        HyprWorkspace workspace = {
                            id : id_int,
                            name : strdup (json_string_value (name)),
                            monitor : strdup (json_string_value (monitor)),
                            monitorID : json_integer_value (monitorID),
                            windows : json_integer_value (windows),
                            hasfullscreen : json_boolean_value (hasfullscreen),
                            lastwindow :
                                strdup (json_string_value (lastwindow)),
                            lastwindowtitle :
                                strdup (json_string_value (lastwindowtitle))
                        };

                        // TODO: stop this from potentially breaking the
                        // program
                        if (id_int >= WORKSPACES_MAX || id_int < 0) {
                            fprintf (
                                stderr,
                                "workspace id %d is greater than the max of "
                                "%d. Can't continue.\n",
                                id_int, WORKSPACES_MAX);
                            fflush (stderr);

                            continue;
                        }

                        addWorkspace (workspace, self, true);

                        printf ("id: %d, name: %s\n",
                                self->workspaces[id_int]->id,
                                self->workspaces[id_int]->name);
                    }
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
onWorkspaceDestroyed (char *payload, HyprBackend *self) {}

void
onWorkspaceCreated (char *payload, HyprBackend *self) {}

void
onMonitorFocused (char *payload, HyprBackend *self) {}

void
onWorkspaceMoved (char *payload, HyprBackend *self) {}

void
onWindowOpened (char *payload, HyprBackend *self) {}

void
onWindowClosed (char *payload, HyprBackend *self) {}

void
onWindowMoved (char *payload, HyprBackend *self) {}

void
setUrgentWorkspace (char *payload, HyprBackend *self) {}

void
onWorkspaceRenamed (char *payload, HyprBackend *self) {}

void
onWindowTitleEvent (char *payload, HyprBackend *self) {}

void
onConfigReloaded (HyprBackend *self) {}