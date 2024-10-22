#pragma once

#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-cursor.h>

#include "hyprland-toplevel-export-v1-client.h"
#include "wlr-foreign-toplevel-management-unstable-v1-client.h"
#include "xdg-shell-client.h"

#include "panel-common.h"

#include "applications-menu/panel-applications-menu.h"

#include <poll.h>
#include <stdbool.h>
#include <sys/time.h>

PanelTaskbar *panel_taskbar_init (Panel *panel);
void panel_taskbar_run (PanelTaskbar *self);
void panel_taskbar_update_monitors (PanelTaskbar *self);
PanelTaskbarWindow *panel_taskbar_window_new (PanelTaskbar *taskbar, GdkMonitor *monitor);