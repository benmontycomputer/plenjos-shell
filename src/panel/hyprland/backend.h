#pragma once

#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// This was helpful:
// https://github.com/Alexays/Waybar/blob/master/src/modules/hyprland/backend.cpp

typedef struct HyprlandBackend {
    int socketfd;
} HyprlandBackend;

HyprlandBackend *hyprland_backend_init ();
void hyprland_backend_run (HyprlandBackend *self);
char *getSocket1Reply (const char *rq);