#include "panel-taskbar-icon.h"

/* // https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
str_split_return_val
str_split (char *a_str, const char a_delim) {
    char **result = 0;
    size_t count = 0;
    char *tmp = a_str;
    char *last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    //printf("%s\n\n", a_str);
    //fflush(stdout);

    /* Count how many elements will be extracted. *//*
    while (*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. *//*
    count += last_comma < (a_str + strlen (a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. *//*
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
}*/


// https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c (but one of the other answers)
str_split_return_val
str_split (char *a_str, const char a_delim) {
    str_split_return_val return_val;
    return_val.result = NULL;
    return_val.count = 0;

    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    char *token = NULL;

    return_val.result = malloc ((return_val.count + 1) * sizeof (char *));

    while ((token = strsep(&a_str, delim))) {
        return_val.count++;

        return_val.result = realloc (return_val.result, (return_val.count + 1) * sizeof (char *));

        return_val.result[return_val.count - 1] = token;
    }

    return_val.result[return_val.count] = NULL;

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

            free (substr);
            free (split.result);
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

    char icon_size_str[32];

    snprintf (icon_size_str, 32, "%d", icon_size);

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
                    if (strstr (paths[i], icon_size_str)
                        || strstr (paths[i], "scalable")) {
                        free (new_path);
                        free_string_list (paths);
                        return icon;
                    }
                }

                free (icon);
            }
        }

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
suggested_icon_for_id (char *id, int icon_size, char *current_theme) {
    if (id[0] == '/' && (access (id, F_OK) == 0)) {
        return g_strdup (id);
    }
    printf ("Icon for %s.\n", id);
    fflush (stdout);

    // char *icon;

    char *xdg_data_dirs = g_strdup (getenv ("XDG_DATA_DIRS"));

    if (!xdg_data_dirs) {
        size_t len = strlen (DATA_DIRS_DEFAULT) + 1;
        xdg_data_dirs = malloc (len);
        snprintf (xdg_data_dirs, len, "%s", DATA_DIRS_DEFAULT);
    }

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

    char *themes[] = { current_theme, "hicolor" };

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
                        free (str_split_result.result);
                        free (xdg_data_dirs);

                        return icon;
                    }
                }
            }
        }
    }

    free (xdg_data_dirs);
    free (str_split_result.result);
    free_string_list (icon_theme_paths);

    return NULL;
}

void
free_icon_exec_map_item (icon_exec_map_item *item) {
    if (item) {
        if (item->exec)
            free (item->exec);
        if (item->icon)
            free (item->icon);

        free (item);
    }
}

int
icon_exec_map_finder (gpointer item, gchar *exec) {
    if (item && exec && !strcmp (((icon_exec_map_item *)item)->exec, exec)) {
        return 0;
    }

    return 1;
}

GList *
init_icon_exec_map () {
    GList *return_val = NULL;

    char *xdg_data_dirs = g_strdup (getenv ("XDG_DATA_DIRS"));

    str_split_return_val str_split_result = str_split (xdg_data_dirs, ':');

    char **desktop_paths
        = malloc ((str_split_result.count + 2) * sizeof (char *));

    for (size_t i = 0; str_split_result.result[i]; i++) {
        size_t len = 0;
        if (str_split_result.result[i]) {
            len = strlen (str_split_result.result[i]);
        }

        size_t path_len = len + strlen ("/applications/") + 1;
        desktop_paths[i] = malloc (path_len);
        snprintf (desktop_paths[i], path_len, "%s/applications/",
                  str_split_result.result[i]);

        desktop_paths[i + 1] = NULL;
    }
    free (str_split_result.result);

    // Don't free.
    const char *home_path = getenv ("HOME");

    size_t home_apps_dir_len
        = strlen (home_path) + strlen ("/.local/share/applications/") + 1;
    desktop_paths[str_split_result.count] = malloc (home_apps_dir_len);

    snprintf (desktop_paths[str_split_result.count], home_apps_dir_len,
              "%s/.local/share/applications/", home_path);
    desktop_paths[str_split_result.count + 1] = NULL;

    str_split_result.count++;

    for (size_t i = 0; i < str_split_result.count; i++) {
        char *dir = desktop_paths[i];

        // https://stackoverflow.com/questions/1271064/how-do-i-loop-through-all-files-in-a-folder-using-c
        struct dirent *dp;
        DIR *dfd;

        if ((dfd = opendir (dir)) != NULL) {
            while ((dp = readdir (dfd)) != NULL) {
                if (dp->d_name[0] != '.') {
                    char *fname_split = g_strdup (dp->d_name);
                    str_split_return_val fname_split_result
                        = str_split (fname_split, '.');

                    if (!strcmp (fname_split_result
                                     .result[fname_split_result.count - 1],
                                 "desktop")) {

                        size_t full_path_len
                            = strlen (dir) + strlen (dp->d_name) + 2;
                        char *full_path = malloc (full_path_len);
                        snprintf (full_path, full_path_len, "%s/%s", dir,
                                  dp->d_name);

                        GKeyFile *kf = g_key_file_new ();
                        g_key_file_load_from_file (kf, full_path,
                                                   G_KEY_FILE_NONE, NULL);
                        free (full_path);

                        icon_exec_map_item *item
                            = malloc (sizeof (icon_exec_map_item));

                        item->exec = g_key_file_get_string (
                            kf, G_KEY_FILE_DESKTOP_GROUP,
                            G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);
                        item->icon = g_key_file_get_string (
                            kf, G_KEY_FILE_DESKTOP_GROUP,
                            G_KEY_FILE_DESKTOP_KEY_ICON, NULL);

                        if (item->exec && item->icon)
                            return_val = g_list_append (return_val, item);
                        else
                            free_icon_exec_map_item (item);

                        icon_exec_map_item *item_2
                            = malloc (sizeof (icon_exec_map_item));

                        item_2->exec = g_strdup (dp->d_name);
                        item_2->exec[strlen (item_2->exec) - 8] = '\0';
                        item_2->icon = g_key_file_get_string (
                            kf, G_KEY_FILE_DESKTOP_GROUP,
                            G_KEY_FILE_DESKTOP_KEY_ICON, NULL);

                        if (item_2->exec && item_2->icon)
                            return_val = g_list_append (return_val, item_2);
                        else
                            free_icon_exec_map_item (item_2);

                        size_t underscore_pos;

                        for (underscore_pos = 0;
                             dp->d_name[underscore_pos] != '\0';
                             underscore_pos++) {
                            if (dp->d_name[underscore_pos] == '_') {
                                icon_exec_map_item *item_3
                                    = malloc (sizeof (icon_exec_map_item));

                                size_t underscore_len
                                    = strlen (dp->d_name) - underscore_pos - 8;
                                char *underscore_str = malloc (underscore_len);
                                snprintf (
                                    underscore_str, underscore_len, "%s",
                                    (char *)(dp->d_name + underscore_pos + 1));

                                item_3->exec = NULL;
                                item_3->icon = NULL;

                                item_3->exec = underscore_str;
                                item_3->icon = g_key_file_get_string (
                                    kf, G_KEY_FILE_DESKTOP_GROUP,
                                    G_KEY_FILE_DESKTOP_KEY_ICON, NULL);

                                if (item_3->exec && item_3->icon)
                                    return_val
                                        = g_list_append (return_val, item_3);
                                else
                                    free_icon_exec_map_item (item_3);

                                break;
                            }
                        }

                        g_key_file_free (kf);
                    }

                    free (fname_split_result.result);
                    free (fname_split);
                }
            }
        }

        closedir (dfd);
    }

    free_string_list (desktop_paths);
    free (xdg_data_dirs);

    return return_val;
}