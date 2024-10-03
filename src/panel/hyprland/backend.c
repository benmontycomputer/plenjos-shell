#include "hyprland/backend.h"

// This was helpful:
// https://github.com/Alexays/Waybar/blob/master/src/modules/hyprland/backend.cpp

// DON'T FREE THE RESULT
static char *
getSocketFolder (const char *instanceSig) {
    // DON'T FREE EVEN IN CALLING FUNCTION
    static char *socketFolder = NULL;

    // socket path, specified by EventManager of Hyprland
    if (socketFolder != NULL) {
        return socketFolder;
    }

    const char *xdgRuntimeDirEnv = getenv ("XDG_RUNTIME_DIR");

    char *path = NULL;

    if (xdgRuntimeDirEnv) {
        DIR *dir = opendir (xdgRuntimeDirEnv);
        if (dir) {
            closedir (dir);

            size_t hypr_path_len = strlen (xdgRuntimeDirEnv) + 6;
            char *hypr_path = malloc (hypr_path_len);

            snprintf (hypr_path, hypr_path_len, "%s/hypr", xdgRuntimeDirEnv);

            dir = opendir (hypr_path);
            if (dir) {
                closedir (dir);

                size_t path_len = hypr_path_len + strlen (instanceSig) + 1;
                path = malloc (path_len);

                snprintf (path, path_len, "%s/%s", hypr_path, instanceSig);
            }

            free (hypr_path);
        } else {
            size_t path_len = strlen ("/tmp/hypr/") + strlen (instanceSig) + 1;
            path = malloc (path_len);

            snprintf (path, path_len, "/tmp/hypr/%s", instanceSig);
        }
    }

    return path;
}

char *
getSocket1Reply (const char *rq) {
    // basically hyprctl

    const int serverSocket = socket (AF_UNIX, SOCK_STREAM, 0);

    if (serverSocket < 0) {
        fprintf (stderr, "Hyprland IPC: Couldn't open a socket (1)\n");
        fflush (stderr);

        return NULL;
    }

    // get the instance signature
    char *instanceSig = getenv ("HYPRLAND_INSTANCE_SIGNATURE");

    if (instanceSig == NULL) {
        fprintf (stderr,
                 "Hyprland IPC: HYPRLAND_INSTANCE_SIGNATURE wasn't set.\n");
        fflush (stderr);

        return NULL;
    }

    struct sockaddr_un serverAddress = { 0 };

    serverAddress.sun_family = AF_UNIX;

    // DON'T FREE
    char *socketFolderPath = getSocketFolder (instanceSig);

    size_t socketPathLen
        = strlen (socketFolderPath) + strlen ("/.socket.sock") + 1;
    char *socketPath = malloc (socketPathLen);

    snprintf (socketPath, socketPathLen, "%s/.socket.sock", socketFolderPath);

    // Use snprintf to copy the socketPath string into serverAddress.sun_path
    if (snprintf (serverAddress.sun_path, sizeof (serverAddress.sun_path),
                  "%s", socketPath)
        < 0) {
        fprintf (stderr, "Hyprland IPC: Couldn't copy socket path (6).\n");
        fflush (stderr);

        return NULL;
    }

    if (connect (serverSocket, (struct sockaddr *)(&serverAddress),
                 sizeof (serverAddress))
        < 0) {
        fprintf (stderr, "Hyprland IPC: Couldn't connect to %s. (3).\n",
                 socketPath);
        fflush (stderr);

        return NULL;
    }

    ssize_t sizeWritten = write (serverSocket, rq, strlen (rq));

    if (sizeWritten < 0) {
        fprintf (stderr, "Hyprland IPC: Couldn't write (4) \n");
        fflush (stderr);

        return NULL;
    }

    char buffer[8192] = { 0 };
    char *response = NULL;

    size_t response_len = 1;

    do {
        sizeWritten = read (serverSocket, buffer, 8192);

        if (sizeWritten < 0) {
            fprintf (stderr, "Hyprland IPC: Couldn't read (5) \n");
            fflush (stderr);
            close (serverSocket);
            return "";
        }

        if (sizeWritten != 0) {
            size_t new_start = response_len - 1;

            response_len += strlen (buffer);

            if (response)
                response = realloc (response, response_len);
            else
                response = malloc (response_len);

            snprintf (response + new_start, response_len - new_start, "%s",
                      buffer);
        }
    } while (sizeWritten > 0);

    close (serverSocket);

    printf ("Opened buffer: \n\n%s\n\n", buffer);
    fflush (stdout);

    return response;
}

void
hyprland_backend_run (HyprlandBackend *self) {
    for (size_t i = 0; i < WORKSPACES_MAX; i++) {
        self->workspaces[i] = NULL;
    }

    FILE *file = fdopen (self->socketfd, "r");

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
            self->workspacesCount = json_array_size (root);

            for (size_t i = 0; i < self->workspacesCount; i++) {
                json_t *data, *id, *name, *monitor, *monitorID, *windows,
                    *hasfullscreen, *lastwindow, *lastwindowtitle;

                const char *message_text;

                data = json_array_get (root, i);
                if (!json_is_object (data)) {
                    fprintf (
                        stderr,
                        "error: workspace data at index %d is not an object\n",
                        i);
                    fflush (stderr);
                } else {
                    // TODO: check types on these

                    id = json_object_get (data, "id");
                    name = json_object_get (data, "name");
                    monitor = json_object_get (data, "monitor");
                    monitorID = json_object_get (data, "monitorID");
                    windows = json_object_get (data, "windows");
                    hasfullscreen = json_object_get (data, "hasfullscreen");
                    lastwindow = json_object_get (data, "lastwindow");
                    lastwindowtitle
                        = json_object_get (data, "lastwindowtitle");

                    int id_int = json_integer_value (id);

                    HyprlandWorkspace workspace = {
                        id : id_int,
                        name : json_string_value (name),
                        monitor : json_string_value (monitor),
                        monitorID : json_integer_value (monitorID),
                        windows : json_integer_value (windows),
                        hasfullscreen : json_boolean_value (hasfullscreen),
                        lastwindow : json_string_value (lastwindow),
                        lastwindowtitle : json_string_value (lastwindowtitle)
                    };

                    // TODO: stop this from potentially breaking the program
                    if (id >= WORKSPACES_MAX) {
                        fprintf (stderr,
                                 "workspace id %d is greater than the max of "
                                 "%d. Can't continue.\n",
                                 id_int, WORKSPACES_MAX);
                        fflush (stderr);

                        continue;
                    }

                    self->workspaces[id_int]
                        = malloc (sizeof (HyprlandWorkspace));
                    self->workspaces[id_int][0] = workspace;

                    printf ("id: %d, name: %s\n", self->workspaces[id_int]->id,
                            self->workspaces[id_int]->name);
                }
            }
        }

        json_decref (root);
    }

    while (true) {
        char buffer[1024];

        char *receivedCharPtr = fgets (buffer, 1024, file);

        if (receivedCharPtr == NULL) {
            usleep (1);

            continue;
        }

        // Using "createworkspace>" instead of "createworkspace" because it's shorter than
        // "createworkspace>>" but "createworkspace" would also match "createworkspacev2>>"
        if (!strncmp (receivedCharPtr, "createworkspace>", 10)) {
            long workspace_id = strtol (receivedCharPtr + 11, NULL, 10);

            if (self->workspaces[workspace_id]) {
                // Show workspace

                fprintf (stderr, "Newly created workspace %d was already in the array. NOT GOOD.\n", (int)workspace_id);
                fflush (stdout);
            } else {
                HyprlandWorkspace workspace;
                workspace.id = (int)workspace_id;
                workspace.
            }
        }

        printf ("%s", receivedCharPtr);
        fflush (stdout);
    }
}

HyprlandBackend *
hyprland_backend_init () {
    const char *his = getenv ("HYPRLAND_INSTANCE_SIGNATURE");

    if (his == NULL) {
        fprintf (stderr, "Hyprland is not running.\n");
        fflush (stderr);

        return NULL;
    }

    struct sockaddr_un addr;
    int socketfd = socket (AF_UNIX, SOCK_STREAM, 0);

    if (socketfd == -1) {
        fprintf (stderr, "Hyprland IPC sockedfd failed.\n");
        fflush (stderr);

        return NULL;
    }

    addr.sun_family = AF_UNIX;

    // DON'T FREE
    char *socketFolderPath = getSocketFolder (his);

    size_t socketPathLen
        = strlen (socketFolderPath) + strlen ("/.socket2.sock") + 1;
    char *socketPath = malloc (socketPathLen);

    snprintf (socketPath, socketPathLen, "%s/.socket2.sock", socketFolderPath);

    strncpy (addr.sun_path, socketPath, sizeof (addr.sun_path) - 1);

    addr.sun_path[sizeof (addr.sun_path) - 1] = 0;

    int l = sizeof (struct sockaddr_un);

    if (connect (socketfd, (struct sockaddr *)&addr, l) == -1) {
        fprintf (stderr, "Hyprland IPC: unable to connect?.\n");
        fflush (stderr);

        return NULL;
    }

    HyprlandBackend *self = malloc (sizeof (HyprlandBackend));

    self->socketfd = socketfd;

    return self;
}