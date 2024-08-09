#include <glib.h>

#include <stdio.h>
#include <stdbool.h>

#include <wayland-client.h>
#include <wayland-client-protocol.h>

#include "wlr-foreign-toplevel-management-unstable-v1-client.h"

typedef struct PanelInterfaceToplevelButton {
    struct zwlr_foreign_toplevel_handle_v1 *m_toplevel_handle;
    char *m_title, m_id;
    struct wl_output *m_output;
    struct wl_array *m_state;
    struct wl_seat *m_seat;
    //GList *m_toplevels;
    bool m_maximized, m_activated, m_minimized, m_fullscreen;
} PanelInterfaceToplevelButton;

PanelInterfaceToplevelButton *panel_interface_toplevel_button_new (struct zwlr_foreign_toplevel_handle_v1 *toplevel_handle, struct wl_seat *seat, GList *toplevels);