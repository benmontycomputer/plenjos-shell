#include <gtk-layer-shell.h>
#include <gtk/gtk.h>

#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-cursor.h>

#include <glib.h>

#include "taskbar/panel-taskbar.h"
#include "clock/panel-clock.h"

#include "applications-menu/panel-applications-menu.h"

#define UNUSED(x) (void)(x)