#include <glib.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#include <wayland-client-protocol.h>
#include <wayland-client.h>

#include "wlr-foreign-toplevel-management-unstable-v1-client.h"

#include "panel-taskbar-icon.h"
#include "panel-taskbar.h"

typedef struct PanelTaskbarToplevelButton {
    struct zwlr_foreign_toplevel_handle_v1 *m_toplevel_handle;
    char *m_title, *m_id, *m_icon_path;
    struct wl_output *m_output;
    struct wl_array *m_state;
    struct wl_seat *m_seat;
    // GList *m_toplevels;
    struct PanelTaskbar *m_taskbar;
    bool m_maximized, m_activated, m_minimized, m_fullscreen;

    // Button
    GtkWidget *rendered;

    GtkWidget *box;

    GtkWidget *icon;
    GtkWidget *label;
} PanelTaskbarToplevelButton;

PanelTaskbarToplevelButton *panel_taskbar_toplevel_button_new (
    struct zwlr_foreign_toplevel_handle_v1 *toplevel_handle,
    struct wl_seat *seat, PanelTaskbar *taskbar);

void panel_taskbar_toplevel_button_rerender (PanelTaskbarToplevelButton *self,
                                             bool update_title,
                                             bool update_app_id_and_icon);

void panel_taskbar_toplevel_button_gtk_run (PanelTaskbarToplevelButton *self);