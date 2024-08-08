#include "panel-interface.h"

typedef struct {
    struct wl_display *display;

    struct wl_registry *registry;

    struct wl_compositor *compositor;
} PanelInterface;

static void
registry_handle_global (PanelInterface *self, struct wl_registry *registry,
                        uint32_t name, const char *interface,
                        uint32_t version) {
    // Add/handle global

    printf ("Found interface %s version %lu\n", interface,
            (unsigned long)version);
    fflush (stdout);

    if (!strcmp(interface, wl_compositor_interface.name)) {
        wl_registry_bind (self->registry, name, self->compositor, version);
        printf("Binding interface %s\n", wl_compositor_interface.name);
        fflush(stdout);
    } else if (interface == )
}

static void
registry_handle_global_remove (PanelInterface *self, struct wl_registry *registry,
                               uint32_t name) {
    // Remove global
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

struct PanelInterface *
panel_interface_init () {
    PanelInterface *self = malloc (sizeof (PanelInterface));

    const char *wl_display_name = getenv ("WAYLAND_DISPLAY");

    if (!wl_display_name) {
        fprintf (stderr, "Error: $WAYLAND_DISPLAY is not set.\n");
        fflush (stderr);
        return NULL;
    } else {
        printf ("Opening Wayland display %s\n", wl_display_name);
        fflush (stdout);
    }

    self->display = wl_display_connect (wl_display_name);

    self->registry = wl_display_get_registry (self->display);

    wl_registry_add_listener (self->registry, &registry_listener,
                              self);

    printf("First roundtrip has started\n");
    fflush(stdout);
    wl_display_roundtrip(self->display);
    printf("First roundtrip has finished\n");
    fflush(stdout);


}