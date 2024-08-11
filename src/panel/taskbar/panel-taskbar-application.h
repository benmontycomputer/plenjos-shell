#pragma once

#include <glib.h>

#include "panel-taskbar.h"

typedef struct {
    GList *toplevels;
    PanelTaskbar *taskbar;

    char *id;

    GtkBox *items_box;
} PanelTaskbarApplication;

typedef struct PanelTaskbarToplevelButton {
    struct zwlr_foreign_toplevel_handle_v1 *m_toplevel_handle;
    char *m_title, *m_id, *m_icon_path;
    struct wl_output *m_output;
    struct wl_array *m_state;
    struct wl_seat *m_seat;
    // GList *m_toplevels;
    PanelTaskbar *m_taskbar;
    PanelTaskbarApplication *m_application;
    bool m_maximized, m_activated, m_minimized, m_fullscreen;

    // Button
    GtkWidget *rendered;

    GtkWidget *box;

    GtkWidget *icon;
    GtkWidget *label;
} PanelTaskbarToplevelButton;

PanelTaskbarApplication *panel_taskbar_application_new (char *id, PanelTaskbar *taskbar);

void panel_taskbar_application_add_toplevel (PanelTaskbarApplication *self, PanelTaskbarToplevelButton *toplevel);

void panel_taskbar_application_remove_toplevel (PanelTaskbarApplication *self, PanelTaskbarToplevelButton *toplevel);