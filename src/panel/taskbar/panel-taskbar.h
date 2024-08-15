#pragma once

#include <gtk-layer-shell.h>
#include <gtk/gtk.h>

#include <wayland-client.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-cursor.h>
#include "xdg-shell-client.h"
#include "wlr-foreign-toplevel-management-unstable-v1-client.h"

#include <stdbool.h>
#include <poll.h>
#include <sys/time.h>

#define UNUSED(x) (void)(x)

typedef struct PanelTaskbar {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct xdg_wm_base *wm_base;
    struct wl_seat *seat;
    struct wl_shm *shm;
    // struct zwlr_layer_shell_v1 *layer_shell;
    struct zwlr_foreign_toplevel_manager_v1 *toplevel_manager;
    struct wl_output *output;

    bool has_keyboard;
    bool has_pointer;

    bool running;

    struct wl_pointer *pointer;
    struct wl_keyboard *keyboard;

    GList *applications;
    GtkBox *taskbar_box;

    GSettings *settings;
} PanelTaskbar;

PanelTaskbar *panel_taskbar_init();
void panel_taskbar_run (PanelTaskbar *self);