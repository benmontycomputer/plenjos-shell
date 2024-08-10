#include <glib.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#include <wayland-client-protocol.h>
#include <wayland-client.h>

#include "wlr-foreign-toplevel-management-unstable-v1-client.h"

#include "panel-icon.h"
#include "panel-interface.h"

typedef struct PanelInterfaceToplevelButton {
    struct zwlr_foreign_toplevel_handle_v1 *m_toplevel_handle;
    char *m_title, *m_id, *m_icon_path;
    struct wl_output *m_output;
    struct wl_array *m_state;
    struct wl_seat *m_seat;
    // GList *m_toplevels;
    struct PanelInterface *m_interface;
    bool m_maximized, m_activated, m_minimized, m_fullscreen;

    GtkWidget *m_rendered;
    GtkWidget *icon;

    GtkWidget *button;
} PanelInterfaceToplevelButton;

PanelInterfaceToplevelButton *panel_interface_toplevel_button_new (
    struct zwlr_foreign_toplevel_handle_v1 *toplevel_handle,
    struct wl_seat *seat, PanelInterface *interface);

void
panel_interface_toplevel_button_rerender (PanelInterfaceToplevelButton *self,
                                          bool update_title,
                                          bool update_app_id_and_icon);