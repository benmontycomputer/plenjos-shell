#include <gtk-layer-shell.h>
#include <gtk/gtk.h>

#include <wayland-client.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-cursor.h>
#include "xdg-shell-client.h"

#include <stdbool.h>
#include <poll.h>
#include <sys/time.h>

typedef struct {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct xdg_wm_base *wm_base;
    struct wl_seat *seat;
    struct wl_shm *shm;
    // struct zwlr_layer_shell_v1 *layer_shell;
    // struct zwlr_foreign_toplevel_manager_v1 *toplevel_manager;
    struct wl_output *output;

    bool has_keyboard;
    bool has_pointer;

    bool running;

    struct wl_pointer *pointer;
    struct wl_keyboard *keyboard;
} PanelInterface;

struct PanelInterface *panel_interface_init();
static void panel_interface_run (PanelInterface *self);