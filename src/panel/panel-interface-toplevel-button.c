#include "panel-interface-toplevel-button.h"

static void
toplevel_handle_title (
    PanelInterfaceToplevelButton *self,
    struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1,
    const char *title) {
    self->m_title = title;
    printf ("%s\n", title);
    fflush (stdout);
}

static void
toplevel_handle_app_id (
    PanelInterfaceToplevelButton *self,
    struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1,
    const char *id) {
    self->m_id = id;
    char *icon = NULL;

    // Is this icon already loaded?
    // TODO: actually check if icon is already loaded
    if (icon == NULL || icon == "") {
        icon = suggested_icon_for_id (id);
    }
}

static void toplevel_excess () {

}

static const struct zwlr_foreign_toplevel_handle_v1_listener toplevel_listener
    = {
          .title = toplevel_handle_title,
          .app_id = toplevel_handle_app_id,
          .closed = toplevel_excess,
          .done = toplevel_excess,
          .output_enter = toplevel_excess,
          .output_leave = toplevel_excess,
          .parent = toplevel_excess,
          .state = toplevel_excess,
      };

PanelInterfaceToplevelButton *
panel_interface_toplevel_button_new (
    struct zwlr_foreign_toplevel_handle_v1 *toplevel_handle,
    struct wl_seat *seat, GList *toplevels) {
    PanelInterfaceToplevelButton *self
        = malloc (sizeof (PanelInterfaceToplevelButton));

    //self->m_toplevels = toplevels;
    self->m_toplevel_handle = toplevel_handle;
    self->m_seat = seat;
    self->m_maximized = self->m_activated = self->m_minimized
        = self->m_fullscreen = false;

    zwlr_foreign_toplevel_handle_v1_add_listener (self->m_toplevel_handle,
                                                  &toplevel_listener, self);
}