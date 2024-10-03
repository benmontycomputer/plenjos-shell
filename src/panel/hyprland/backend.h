#pragma once

#include <dirent.h>
#include <errno.h>
#include <jansson.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define WORKSPACES_MAX 256

// This was helpful:
// https://github.com/Alexays/Waybar/blob/master/src/modules/hyprland/backend.cpp

typedef struct HyprlandWorkspace {
    int id;
    char *name;
    char *monitor;
    int monitorID;
    int windows;
    bool hasfullscreen;
    char *lastwindow;
    char *lastwindowtitle;
} HyprlandWorkspace;

typedef struct HyprlandBackend {
    int socketfd;

    int workspacesCount;
    HyprlandWorkspace *workspaces[WORKSPACES_MAX];
} HyprlandBackend;

HyprlandBackend *hyprland_backend_init ();
void hyprland_backend_run (HyprlandBackend *self);
char *getSocket1Reply (const char *rq);