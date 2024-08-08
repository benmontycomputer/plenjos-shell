#include "panel-interface.h"

typedef struct {
    struct wl_display *display;
} PanelInterface;

struct PanelInterface *panel_interface_init() {
    PanelInterface *interface = malloc(sizeof(PanelInterface));

    const char *wl_display_name = getenv("WAYLAND_DISPLAY");

    if (!wl_display_name) {
        fprintf(stderr, "Error: $WAYLAND_DISPLAY is not set.\n");
        fflush(stderr);
        return NULL;
    } else {
        printf("Opening Wayland display %s\n", wl_display_name);
        fflush(stdout);
    }

    interface->display = wl_display_connect(wl_display_name);

    wl_display_get_registry(interface->display);
}