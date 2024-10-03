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
#define MONITORS_MAX 256

// This was helpful:
// https://github.com/Alexays/Waybar/blob/master/src/modules/hyprland/backend.cpp

typedef struct HyprWorkspace {
    int id;
    char *name;
    char *monitor;
    int monitorID;
    int windows;
    bool hasfullscreen;
    char *lastwindow;
    char *lastwindowtitle;
} HyprWorkspace;

typedef struct HyprMonitor {
    int id;
    char *name;
    int currentWorkspace;
} HyprMonitor;

typedef struct HyprBackend {
    int socketfd;

    HyprWorkspace *workspaces[WORKSPACES_MAX];
    HyprMonitor *monitors[MONITORS_MAX];

    int currentMonitor;
} HyprBackend;

HyprBackend *hypr_backend_init ();
void hypr_backend_run (HyprBackend *self);
char *getSocket1Reply (const char *rq);