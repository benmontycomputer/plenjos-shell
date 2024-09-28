#include <gtk4-layer-shell.h>
#include <gtk/gtk.h>

#define UNUSED(x) (void)(x)

typedef struct {
    GListModel *monitors;

    GtkWindow **windows;

    GtkApplication *app;
} Desktop;