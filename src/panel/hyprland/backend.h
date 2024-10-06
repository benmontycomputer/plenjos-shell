#pragma once

#include <dirent.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <jansson.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <wayland-client.h>

#include "hyprland-toplevel-export-v1-client.h"

#include "panel-common.h"

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

    GdkMonitor *gdk_monitor;

    GtkWidget *button;
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

    Panel *panel;
    struct hyprland_toplevel_export_manager_v1 *toplevelExportManager;

    bool backend_waiting;
} HyprBackend;

HyprBackend *hypr_backend_init (Panel *panel);
void hypr_backend_run (HyprBackend *self);
char *getSocket1Reply (const char *rq);