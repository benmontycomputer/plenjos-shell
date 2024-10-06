#include "hyprland/backend.h"
#include "hyprland/workspaces.h"

struct hyprland_toplevel_export_frame_v1 *getToplevelFrame (struct zwlr_foreign_toplevel_handle_v1 *handle, HyprBackend *self) {
    struct hyprland_toplevel_export_frame_v1 *frame = hyprland_toplevel_export_manager_v1_capture_toplevel_with_wlr_toplevel_handle (self->panel->taskbar->export_manager, false, handle);

    struct wl_buffer buffer;

    hyprland_toplevel_exportframe
}

// This was helpful:
// https://github.com/Alexays/Waybar/blob/master/src/modules/hyprland/backend.cpp

// DON'T FREE THE RESULT
static char *
getSocketFolder (const char *instanceSig) {
    // DON'T FREE EVEN IN CALLING FUNCTION
    static char *path = NULL;

    // socket path, specified by EventManager of Hyprland
    if (path != NULL) {
        return path;
    }

    const char *xdgRuntimeDirEnv = getenv ("XDG_RUNTIME_DIR");

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

        free (socketPath);

        return NULL;
    }

    if (connect (serverSocket, (struct sockaddr *)(&serverAddress),
                 sizeof (serverAddress))
        < 0) {
        fprintf (stderr, "Hyprland IPC: Couldn't connect to %s. (3).\n",
                 socketPath);
        fflush (stderr);

        free (socketPath);

        return NULL;
    }

    free (socketPath);

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

    return response;
}

void
hypr_backend_run (HyprBackend *self) {
    for (size_t i = 0; i < WORKSPACES_MAX; i++) {
        self->workspaces[i] = NULL;
    }

    updateWorkspaces (self);

    FILE *file = fdopen (self->socketfd, "r");

    self->backend_waiting = false;

    while (true) {
        while (self->backend_waiting) {
            usleep (1);
        }

        char buffer[1024];

        char *receivedCharPtr = fgets (buffer, 1024, file);

        if (receivedCharPtr == NULL) {
            usleep (1);

            continue;
        }

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
hypr_backend_init (Panel *panel) {
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

    free (socketPath);

    addr.sun_path[sizeof (addr.sun_path) - 1] = 0;

    int l = sizeof (struct sockaddr_un);

    if (connect (socketfd, (struct sockaddr *)&addr, l) == -1) {
        fprintf (stderr, "Hyprland IPC: unable to connect?.\n");
        fflush (stderr);

        return NULL;
    }

    HyprBackend *self = malloc (sizeof (HyprBackend));

    self->socketfd = socketfd;
    self->currentMonitor = 0;
    self->panel = panel;

    return self;
}