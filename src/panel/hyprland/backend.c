#include "hyprland/backend.h"
#include "hyprland/workspaces.h"

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
hypr_backend_run (HyprBackend *self) {
    for (size_t i = 0; i < WORKSPACES_MAX; i++) {
        self->workspaces[i] = NULL;
    }

    updateWorkspace (UPDATE_ALL_WORKSPACES, self);

    FILE *file = fdopen (self->socketfd, "r");

    while (true) {
        char buffer[1024];

        char *receivedCharPtr = fgets (buffer, 1024, file);

        if (receivedCharPtr == NULL) {
            usleep (1);

            continue;
        }

        // Using "createworkspace>" instead of "createworkspace" because it's
        // shorter than "createworkspace>>" but "createworkspace" would also
        // match "createworkspacev2>>"
        /* if (!strncmp (receivedCharPtr, "createworkspace>", 16)) {
            long workspace_id = strtol (receivedCharPtr + 17, NULL, 10);

            if (self->workspaces[workspace_id]) {
                // Show workspace

                fprintf (stderr, "Newly created workspace %d was already in the
        array. NOT GOOD.\n", (int)workspace_id); fflush (stdout); } else {
                HyprWorkspace workspace;
                workspace.id = (int)workspace_id;
                workspace.monitorID = self->monitors[self->currentMonitor];
            }
        } */

        if (!strncmp (receivedCharPtr, "workspace>", 10)) {
            onWorkspaceActivated (receivedCharPtr + 11, self);
        } else if (!strncmp (receivedCharPtr, "activespecial>", 14)) {
            onSpecialWorkspaceActivated (receivedCharPtr + 15, self);
        } else if (!strncmp (receivedCharPtr, "destroyworkspace>", 17)) {
            onWorkspaceDestroyed (receivedCharPtr + 18, self);
        } else if (!strncmp (receivedCharPtr, "createworkspace>", 16)) {
            onWorkspaceCreated (receivedCharPtr + 17, self);
        } else if (!strncmp (receivedCharPtr, "focusedmon>", 11)) {
            onMonitorFocused (receivedCharPtr + 12, self);
        } else if (!strncmp (receivedCharPtr, "moveworkspace>", 14)) {
            onWorkspaceMoved (receivedCharPtr + 15, self);
        } else if (!strncmp (receivedCharPtr, "openwindow>", 11)) {
            onWindowOpened (receivedCharPtr + 12, self);
        } else if (!strncmp (receivedCharPtr, "closewindow>", 12)) {
            onWindowClosed (receivedCharPtr + 13, self);
        } else if (!strncmp (receivedCharPtr, "movewindow>", 11)) {
            onWindowMoved (receivedCharPtr + 12, self);
        } else if (!strncmp (receivedCharPtr, "urgent>", 7)) {
            setUrgentWorkspace (receivedCharPtr + 8, self);
        } else if (!strncmp (receivedCharPtr, "renameworkspace>", 16)) {
            onWorkspaceRenamed (receivedCharPtr + 17, self);
        } else if (!strncmp (receivedCharPtr, "windowtitle>", 12)) {
            onWindowTitleEvent (receivedCharPtr + 13, self);
        } else if (!strncmp (receivedCharPtr, "configreloaded>", 15)) {
            onConfigReloaded (self);
        }

        printf ("%s", receivedCharPtr);
        fflush (stdout);
    }
}

HyprBackend *
hypr_backend_init () {
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

    HyprBackend *self = malloc (sizeof (HyprBackend));

    self->socketfd = socketfd;

    return self;
}