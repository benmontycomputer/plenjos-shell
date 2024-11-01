#pragma once

#include <gtk/gtk.h>

#include "applications-menu/panel-applications-menu.h"

#define DASHBOARD_LAUNCH_STR "__PANEL_DASHBOARD__"

enum TaskbarStyle {
    TASKBAR_STYLE_THREE_D_DOCK,
    TASKBAR_STYLE_DOCK,
    TASKBAR_STYLE_PANEL,
    TASKBAR_STYLE_TRAY,
    TASKBAR_STYLE_INVISIBLE
};

typedef struct {
    struct PanelTaskbar *taskbar;
    gpointer *tray;

    bool supports_alpha;
    bool dark_mode;

    GSettings *panel_settings;

    GtkApplication *app;

    GListModel *monitors;
} Panel;

typedef struct PanelTaskbarWindow {
    GtkWindow *gtk_window;
    GtkBox *taskbar_box;

    GdkMonitor *monitor;

    enum TaskbarStyle style;

    GdkRGBA bg_primary;
    GdkRGBA bg_primary_bottom;
    GdkRGBA bg_secondary;
    GdkRGBA border_primary;
    GdkRGBA border_secondary;

    struct PanelTaskbar *taskbar;
    PanelApplicationsMenu *apps_menu;
} PanelTaskbarWindow;

typedef struct PanelTaskbar {
    PanelTaskbarWindow **windows;

    Panel *panel;

    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct xdg_wm_base *wm_base;
    struct wl_seat *seat;
    struct wl_shm *shm;
    // struct zwlr_layer_shell_v1 *layer_shell;
    struct zwlr_foreign_toplevel_manager_v1 *toplevel_manager;
    struct hyprland_toplevel_export_manager_v1 *export_manager;
    struct wl_output *output;

    bool has_keyboard;
    bool has_pointer;

    bool running;

    struct wl_pointer *pointer;
    struct wl_keyboard *keyboard;

    GList *applications;

    GSettings *settings;

} PanelTaskbar;

#define UNUSED(x) (void)(x)

#define DATA_DIRS_DEFAULT "/usr/share:/usr/local/share"