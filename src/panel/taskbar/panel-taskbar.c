#include "panel-taskbar.h"
#include "panel-common.h"
#include "panel-taskbar-application.h"
#include "panel-taskbar-toplevel-button.h"

void
panel_taskbar_update_monitors (PanelTaskbar *self) {
    if (self->windows) {
        for (size_t i = 0; self->windows[i]; i++) {
            PanelTaskbarWindow *win = self->windows[i];

            gtk_window_close (win->gtk_window);

            free (win);
        }

        free (self->windows);
    }

    size_t n_monitors
        = g_list_model_get_n_items (self->panel->monitors);

    self->windows = malloc ((n_monitors + 1) * sizeof (PanelTaskbarWindow *));

    for (size_t i = 0; i < n_monitors; i++) {
        self->windows[i] = panel_taskbar_window_new (
            self, GDK_MONITOR (g_list_model_get_item (
                      ((Panel *)self->panel)->monitors, i)));
    }

    self->windows[n_monitors] = NULL;

    for (GList *list = self->applications; list = list->next; list) {
        panel_taskbar_application_update_monitors ((PanelTaskbarApplication *)list->data);
    }
}

// Lots of code adapted from https://github.com/selairi/yatbfw

static void
toplevel_manager_handle_toplevel (
    void *data, struct zwlr_foreign_toplevel_manager_v1 *toplevel_manager,
    struct zwlr_foreign_toplevel_handle_v1 *toplevel_handle) {
    UNUSED (toplevel_manager);

    PanelTaskbar *self = (PanelTaskbar *)data;

    panel_taskbar_toplevel_button_new (toplevel_handle, self->seat, self);
}

static const struct zwlr_foreign_toplevel_manager_v1_listener
    toplevel_manager_listener
    = {
          .toplevel = toplevel_manager_handle_toplevel,
      };

static void
registry_handle_global (void *data, struct wl_registry *registry,
                        uint32_t name, const char *interface,
                        uint32_t version) {
    // Add/handle global

    PanelTaskbar *self = (PanelTaskbar *)data;

    UNUSED (registry);

    if (!strcmp (interface, wl_compositor_interface.name)) {
        self->compositor = wl_registry_bind (
            self->registry, name, &wl_compositor_interface, version);
        printf ("Binding interface %s\n", wl_compositor_interface.name);
        fflush (stdout);
    } else if (!strcmp (interface, xdg_wm_base_interface.name)) {
        self->wm_base = wl_registry_bind (self->registry, name,
                                          &xdg_wm_base_interface, version);
        printf ("Binding interface %s\n", xdg_wm_base_interface.name);
        fflush (stdout);
    } else if (!strcmp (interface, wl_seat_interface.name)) {
        self->seat = wl_registry_bind (self->registry, name,
                                       &wl_seat_interface, version);
        printf ("Binding interface %s\n", wl_seat_interface.name);
        fflush (stdout);
    } else if (!strcmp (interface, wl_shm_interface.name)) {
        self->shm = wl_registry_bind (self->registry, name, &wl_shm_interface,
                                      version);
        printf ("Binding interface %s\n", wl_shm_interface.name);
        fflush (stdout);
    } /* else if (!strcmp (interface, zwlr_layer_shell_v1_interface.name)) {
        self->layer_shell = wl_registry_bind (
            self->registry, name, &zwlr_layer_shell_v1_interface, version);
        printf ("Binding interface %s\n", zwlr_layer_shell_v1_interface.name);
        fflush (stdout);
    } */
    else if (!strcmp (interface,
                      zwlr_foreign_toplevel_manager_v1_interface.name)) {
        self->toplevel_manager = wl_registry_bind (
            self->registry, name, &zwlr_foreign_toplevel_manager_v1_interface,
            version);
        printf ("Binding interface %s\n",
                zwlr_foreign_toplevel_manager_v1_interface.name);
        fflush (stdout);
        zwlr_foreign_toplevel_manager_v1_add_listener (
            self->toplevel_manager, &toplevel_manager_listener, self);
    } else if (!strcmp (interface,
                        hyprland_toplevel_export_manager_v1_interface.name)) {
        self->export_manager = wl_registry_bind (
            self->registry, name,
            &hyprland_toplevel_export_manager_v1_interface, version);
        printf ("Binding interface %s\n",
                hyprland_toplevel_export_manager_v1_interface.name);
        fflush (stdout);
    } else if (!strcmp (interface, wl_output_interface.name)) {
        self->output = wl_registry_bind (self->registry, name,
                                         &wl_output_interface, version);
        printf ("Binding interface %s\n", wl_output_interface.name);
        fflush (stdout);
    }
}

static void
registry_handle_global_remove (void *data, struct wl_registry *registry,
                               uint32_t name) {
    // Remove global

    PanelTaskbar *self = (PanelTaskbar *)data;

    UNUSED (self);
    UNUSED (registry);
    UNUSED (name);
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

static void
seat_handle_capabilities (void *data, struct wl_seat *wl_seat,
                          uint32_t capabilities) {
    PanelTaskbar *self = (PanelTaskbar *)data;

    UNUSED (wl_seat);

    self->has_keyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;
    self->has_pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;
    printf ("Keyboard: %s, pointer: %s\n",
            self->has_keyboard ? "true" : "false",
            self->has_pointer ? "true" : "false");
    fflush (stdout);
}

static void
wl_seat_name (void *data, struct wl_seat *seat, const char *name) {
    UNUSED (data);
    UNUSED (seat);
    UNUSED (name);
}

static const struct wl_seat_listener seat_listener = {
    .capabilities = seat_handle_capabilities,
    .name = wl_seat_name,
};

PanelTaskbar *
panel_taskbar_init (Panel *panel) {
    PanelTaskbar *self = malloc (sizeof (PanelTaskbar));

    self->windows = NULL;
    self->panel = panel;

    self->settings = g_settings_new ("org.gnome.desktop.interface");

    self->applications = NULL;

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

    wl_registry_add_listener (self->registry, &registry_listener, self);

    printf ("First roundtrip has started\n");
    fflush (stdout);
    wl_display_roundtrip (self->display);
    printf ("First roundtrip has finished\n");
    fflush (stdout);

    if (!self->compositor) {
        fprintf (stderr, "wl_compositor not loaded.");
        fflush (stderr);
        return NULL;
    } else if (!self->seat) {
        fprintf (stderr, "wl_seat not loaded.");
        fflush (stderr);
        return NULL;
    } else if (!self->shm) {
        fprintf (stderr, "wl_shm not loaded.");
        fflush (stderr);
        return NULL;
    } /* else if (!self->layer_shell) {
        fprintf(stderr, "layer_shell not loaded.");
        fflush(stderr);
        return NULL;
    } */
    else if (!self->toplevel_manager) {
        fprintf (stderr, "foreign_toplevel not loaded.");
        fflush (stderr);
        return NULL;
    } else if (!self->output) {
        fprintf (stderr, "wl_output not loaded.");
        fflush (stderr);
        return NULL;
    }

    wl_seat_add_listener (self->seat, &seat_listener, self);

    wl_display_roundtrip (self->display);

    if (!self->has_keyboard) {
        fprintf (stderr, "no keyboard support.");
        fflush (stderr);
        return NULL;
    }
    if (!self->has_pointer) {
        fprintf (stderr, "no pointer support.");
        fflush (stderr);
        return NULL;
    }

    self->pointer = wl_seat_get_pointer (self->seat);
    self->keyboard = wl_seat_get_keyboard (self->seat);

    PanelTaskbarApplication *dashboard = panel_taskbar_application_new ("view-app-grid", self);

    panel_taskbar_application_set_pinned (dashboard, TRUE);

    dashboard->exec = g_strdup (DASHBOARD_LAUNCH_STR);

    self->applications = g_list_append (self->applications, dashboard);

    return self;
}

void
panel_taskbar_run (PanelTaskbar *self) {
    self->running = true;
    struct pollfd fds[1];
    int timeout_msecs = -1;
    int ret;
    long now_in_msecs;

    struct timeval timeval_current;

    fds[0].fd = wl_display_get_fd (self->display);
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    while (self->running) {
        // Update timeout and run timeout events
        gettimeofday (&timeval_current, NULL);
        now_in_msecs = timeval_current.tv_usec / 1000;
        UNUSED (now_in_msecs);
        timeout_msecs = -1;
        // for ()
        //  TODO: finish this part

        // printf ("Timeout %d\n", timeout_msecs);

        // Process pending Wayland events
        wl_display_dispatch_pending (self->display);
        wl_display_flush (self->display);

        // Wait for events
        ret = poll (fds, sizeof (fds) / sizeof (fds[0]), timeout_msecs);
        if (ret > 0) {
            if (fds[0].revents) {
                wl_display_dispatch (self->display);
                fds[0].revents = 0;
                // printf ("Poll %d\n", ret);
                // fflush (stdout);
            }
        } else if (ret == 0) {
            // printf ("Timeout\n");
            // fflush (stdout);
        } else {
            printf ("Poll failed %d\n", ret);
            fflush (stdout);
        }
    }
}