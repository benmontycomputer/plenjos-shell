#include "panel-taskbar.h"
#include "panel-taskbar-application.h"
#include "panel-taskbar-toplevel-button.h"

// Lots of code adapted from https://github.com/selairi/yatbfw

gboolean
handle_toplevel_gtk (PanelTaskbarToplevelButton *toplevel_button) {
    panel_taskbar_toplevel_button_gtk_run (toplevel_button);

    gtk_container_add (
        GTK_CONTAINER (toplevel_button->m_taskbar->taskbar_box),
        toplevel_button->rendered);
    gtk_widget_show_all (
        GTK_WIDGET (toplevel_button->m_taskbar->taskbar_box));

    return FALSE;
}

static void
toplevel_manager_handle_toplevel (
    void *data, struct zwlr_foreign_toplevel_manager_v1 *toplevel_manager,
    struct zwlr_foreign_toplevel_handle_v1 *toplevel_handle) {
    UNUSED (toplevel_manager);

    PanelTaskbar *self = (PanelTaskbar *)data;

    PanelTaskbarToplevelButton *toplevel_button
        = panel_taskbar_toplevel_button_new (toplevel_handle, self->seat,
                                               self);
    // TODO: set width, height, stuff
    //self->toplevel_handles
    //    = g_list_append (self->toplevel_handles, toplevel_button);
    
    GList *applications = self->applications;

    while (applications) {
        PanelTaskbarApplication *application = (PanelTaskbarApplication *)applications->data;

        if (!strcmp (application->id,)

        applications = applications->next;
    }

    gdk_threads_add_idle ((GSourceFunc)handle_toplevel_gtk, toplevel_button);
}

/*static void
panel_taskbar_remove_toplevel (
    PanelTaskbarToplevelButton *self,
    struct zwlr_foreign_toplevel_handle_v1 *toplevel_handle) {
    UNUSED (self);
    UNUSED (toplevel_handle);
}*/

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

    // printf ("Found interface %s version %lu\n", interface,
    //         (unsigned long)version);
    // fflush (stdout);

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
panel_taskbar_init () {
    PanelTaskbar *self = malloc (sizeof (PanelTaskbar));

    self->taskbar_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_widget_set_name (GTK_WIDGET (self->taskbar_box), "taskbar_box");

    self->toplevel_handles = g_list_alloc ();

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

        printf ("Timeout %d\n", timeout_msecs);

        // Process pending Wayland events
        wl_display_dispatch_pending (self->display);
        wl_display_flush (self->display);

        // Wait for events
        ret = poll (fds, sizeof (fds) / sizeof (fds[0]), timeout_msecs);
        if (ret > 0) {
            if (fds[0].revents) {
                wl_display_dispatch (self->display);
                fds[0].revents = 0;
                printf ("Poll %d\n", ret);
                fflush (stdout);
            }
        } else if (ret == 0) {
            printf ("Timeout\n");
            fflush (stdout);
        } else {
            printf ("Poll failed %d\n", ret);
            fflush (stdout);
        }
    }
}