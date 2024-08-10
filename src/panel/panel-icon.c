#include "panel-icon.h"

// https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
str_split_return_val
str_split (char *a_str, const char a_delim) {
    char **result = 0;
    size_t count = 0;
    char *tmp = a_str;
    char *last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen (a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc (sizeof (char *) * count);

    if (result) {
        size_t idx = 0;
        char *token = strtok (a_str, delim);

        while (token) {
            assert (idx < count);
            *(result + idx++) = strdup (token);
            token = strtok (0, delim);
        }
        assert (idx == count - 1);
        *(result + idx) = 0;
    }

    str_split_return_val return_val;
    return_val.result = result;
    return_val.count = count - 1;

    return return_val;
}

void
free_string_list (char **list) {
    size_t i;
    for (i = 0; list[i]; i++) {
        free (list[i]);
    }
    // This is needed to free the NULL pointer marking the end of the list
    free (list[i]);
    free (list);
}

char **
read_index_theme_paths (char *path, int icon_size) {
    UNUSED (icon_size);

    size_t paths_count = 0;
    char **paths = NULL;

    // size_t parents_count = 0;
    // char **parents = NULL;

    // printf ("Reading %s/index.theme\n", path);
    // fflush (stdout);

    // https://stackoverflow.com/questions/3501338/c-read-file-line-by-line
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    size_t new_path_len = strlen (path) + strlen ("/index.theme") + 1;
    char *new_path = malloc (new_path_len);

    snprintf (new_path, new_path_len, "%s/index.theme", path);

    fp = fopen (new_path, "r");
    if (fp == NULL) {
        // fprintf (stderr, "Couldn't open %s.\n", new_path);
        // fflush (stderr);

        free (new_path);

        return NULL;
    }

    GRegex *re_directories = g_regex_new ("^Directories=.*", 0, 0, NULL);
    // GRegex *re_parents = g_regex_new ("^Inherits=.*", 0, 0, NULL);

    while ((read = getline (&line, &len, fp)) != -1) {
        line[len - 1] = '\0';

        if (g_regex_match (re_directories, line, 0, NULL)) {
            char *substr = g_strdup (line + 12);
            str_split_return_val split = str_split (substr, ',');

            for (size_t i = 0; i < split.count; i++) {
                if (paths_count == 0) {
                    paths = malloc (sizeof (char *));
                } else {
                    paths
                        = realloc (paths, sizeof (char *) * (paths_count + 1));
                }

                paths[paths_count] = g_strdup (split.result[i]);

                paths_count++;
            }

            free_string_list (split.result);
            free (substr);
        } /* else if (g_regex_match (re_parents, line, 0, NULL)) {
            str_split_return_val split = str_split (g_strdup (line + 9), ',');

            for (size_t i = 0; i < split.count; i++) {
                if (parents_count == 0) {
                    parents = malloc (sizeof (char *));
                } else {
                    parents = realloc (parents,
                                       sizeof (char *) * (parents_count + 1));
                }

                parents[parents_count] = split.result[i];

                parents_count++;
            }
        }*/
    }

    g_regex_unref (re_directories);
    // g_regex_unref (re_parents);

    fclose (fp);
    if (line)
        free (line);

    free (new_path);

    paths = realloc (paths, sizeof (char *) * (paths_count + 1));
    paths[paths_count] = NULL;

    return paths;
}

char **
categories_from_theme_path (char *path, int icon_size) {
    // char **categories;

    UNUSED (path);
    UNUSED (icon_size);

    return NULL;

    // if ()
}

char *
get_icon_for_theme (char *path, char *theme, char *icon_name, int icon_size) {
    // printf ("path %s theme %s icon %s panel_size %d\n", path, theme,
    // icon_name,
    //         icon_size);
    // fflush (stdout);

    char *formats[] = { ".png", ".svg" };

    char **paths = NULL;

    size_t new_path_len = strlen (path) + strlen (theme) + 2;
    char *new_path = malloc (new_path_len);
    snprintf (new_path, new_path_len, "%s/%s", path, theme);

    // if (!strcmp (theme, "hicolor")) {
    //     paths = categories_from_theme_path (new_path, icon_size);
    // } else {
    paths = read_index_theme_paths (new_path, icon_size);
    //}

    if (paths) {
        for (size_t i = 0; paths[i]; i++) {
            for (int j = 0; j < 2; j++) {
                size_t icon_len = strlen (path) + strlen (theme)
                                  + strlen (paths[i]) + strlen (icon_name)
                                  + strlen (formats[j]) + 4;
                char *icon = malloc (icon_len);
                snprintf (icon, icon_len, "%s/%s/%s/%s%s", path, theme,
                          paths[i], icon_name, formats[j]);

                // printf("testing: %s\n", icon);
                // fflush(stdout);

                if (access (icon, F_OK) == 0) {
                    // printf("ICON: %s\n\n\n", icon);
                    // fflush(stdout);
                    free (new_path);
                    free_string_list (paths);
                    return icon;
                }

                free (icon);
            }
        }

        free_string_list (paths);
    }

    free (new_path);

    return NULL;
}

char *
suggested_icon_for_id (char *id, int icon_size) {
    if (id[0] == '/' && (access (id, F_OK) == 0)) {
        return g_strdup (id);
    }

    // char *icon;

    char *xdg_data_dirs = g_strdup (getenv ("XDG_DATA_DIRS"));

    str_split_return_val str_split_result = str_split (xdg_data_dirs, ':');

    size_t icon_theme_paths_count = str_split_result.count + 1;

    char **icon_theme_paths
        = malloc ((icon_theme_paths_count + 1) * sizeof (char *));

    // Don't free getenv() results
    char *home_path = getenv ("HOME");
    size_t path_0_len = strlen (home_path) + strlen ("/.icons/") + 1;
    icon_theme_paths[0] = malloc (path_0_len);
    snprintf (icon_theme_paths[0], path_0_len, "%s/.icons/", home_path);

    for (size_t i = 0; str_split_result.result[i]; i++) {
        size_t len = 0;
        if (str_split_result.result[i]) {
            len = strlen (str_split_result.result[i]);
        }

        size_t path_len = len + strlen ("/icons/") + 1;
        icon_theme_paths[i + 1] = malloc (path_len);
        snprintf (icon_theme_paths[i + 1], path_len, "%s/icons/",
                  str_split_result.result[i]);

        icon_theme_paths[i + 2] = NULL;
    }

    char *themes[] = { "Yaru", "hicolor" };

    // char *formats[] = { ".png", ".svg" };

    for (size_t i = 0; icon_theme_paths[i]; i++) {
        char *icon_theme_path = icon_theme_paths[i];

        if (icon_theme_path) {
            for (size_t j = 0; j < 2; j++) {
                if (themes[j]) {
                    char *icon = get_icon_for_theme (icon_theme_path,
                                                     themes[j], id, icon_size);

                    if (icon) {
                        free_string_list (icon_theme_paths);
                        free_string_list (str_split_result.result);
                        free (xdg_data_dirs);

                        return icon;
                    }
                }
            }
        }
    }

    free (xdg_data_dirs);
    free_string_list (str_split_result.result);
    free_string_list (icon_theme_paths);

    return NULL;
}