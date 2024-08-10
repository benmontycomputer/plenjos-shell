#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "panel-interface.h"

typedef struct {
    char **result;
    size_t count;
} str_split_return_val;

str_split_return_val str_split (char *a_str, const char a_delim);
void free_string_list (char **list);

char *suggested_icon_for_id (char *id, int icon_size);