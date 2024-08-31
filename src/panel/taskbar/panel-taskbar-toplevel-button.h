#pragma once

#include <glib.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#include <wayland-client-protocol.h>
#include <wayland-client.h>

#include "wlr-foreign-toplevel-management-unstable-v1-client.h"

#include "panel-taskbar-icon.h"
#include "panel-taskbar.h"

#include "panel-taskbar-application.h"

PanelTaskbarToplevelButton *panel_taskbar_toplevel_button_new (
    struct zwlr_foreign_toplevel_handle_v1 *toplevel_handle,
    struct wl_seat *seat, PanelTaskbar *taskbar);

void panel_taskbar_toplevel_button_rerender (PanelTaskbarToplevelButton *self,
                                             bool update_title,
                                             bool update_app_id_and_icon);

void panel_taskbar_toplevel_button_gtk_run (PanelTaskbarToplevelButton *self);

char *get_icon_from_app_id (PanelTaskbar *taskbar, char *id);