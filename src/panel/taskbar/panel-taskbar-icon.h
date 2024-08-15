#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "panel-taskbar.h"

typedef struct {
    char **result;
    size_t count;
} str_split_return_val;

str_split_return_val str_split (char *a_str, const char a_delim);
void free_string_list (char **list);

char *suggested_icon_for_id (char *id, int icon_size, char *current_theme);

typedef struct {
    char *exec;
    char *icon;
} icon_exec_map_item;

GList *init_icon_exec_map ();

void free_icon_exec_map_item (icon_exec_map_item *item);

int icon_exec_map_finder (gpointer item, gchar *exec);