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

    printf("%s\n\n\n", socketPath);
    fflush(stdout);

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

    do {
        sizeWritten = read (serverSocket, buffer, 8192);

        if (sizeWritten < 0) {
            fprintf (stderr, "Hyprland IPC: Couldn't read (5) \n");
            fflush (stderr);
            close (serverSocket);
            return "";
        }

        printf ("%s\n", buffer);
        fflush (stdout);

        response = malloc (sizeWritten + 1);
        snprintf (response, sizeWritten + 1, "%s", buffer);
    } while (sizeWritten > 0);

    close (serverSocket);
    return response;
}

void
hyprland_backend_run (HyprlandBackend *self) {
    FILE *file = fdopen (self->socketfd, "r");

    while (true) {
        char buffer[1024];

        char *receivedCharPtr = fgets (buffer, 1024, file);

        if (receivedCharPtr == NULL) {
            usleep (1);

            continue;
        }

        printf ("%s\n", receivedCharPtr);
        fflush (stdout);

        printf ("Workspaces: %s\n\n", getSocket1Reply ("workspaces"));

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