#include <gtk-layer-shell.h>
#include <gtk/gtk.h>

#include <wayland-client.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-cursor.h>

struct PanelInterface;

struct PanelInterface *panel_interface_init();