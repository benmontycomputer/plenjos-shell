#include <gtk4-layer-shell.h>
#include <gtk/gtk.h>

#define UNUSED(x) (void)(x)

typedef struct {
    GListModel *monitors;
    GtkWindow **windows;

    char *bg;
    GdkPixbuf *bg_pbuf;

    GtkApplication *app;
} Desktop;

typedef struct {
    GtkPicture *picture;
    GSettings *interface_settings;
} DesktopWindow;