#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gio/gio.h>
#include <glib.h>

GSettings *settings, *settings_a11y;

static void on_change(GFile *file) {
    gchar *value = NULL;
    if (!g_file_load_contents(file, NULL, &value, NULL, NULL, NULL)) {
        return;
    }
    g_strstrip(value);

    gchar *key = g_file_get_basename(file);

    if (g_strcmp0(key, "color-scheme") == 0) {
        if (g_strcmp0(value, "dark") == 0) {
            g_settings_set_string(settings, "color-scheme", "prefer-dark");
        } else if (g_strcmp0(value, "light") == 0) {
            g_settings_set_string(settings, "color-scheme", "prefer-light");
        } else {
            g_settings_set_string(settings, "color-scheme", "default");
        }
    } else if (g_strcmp0(key, "accent-color") == 0) {
        // TODO: translate color to color name
        g_settings_set_string(settings, "accent-color", value);
    } else if (g_strcmp0(key, "reduced-motion") == 0) {
        if (g_strcmp0(value, "reduced") == 0) {
            g_settings_set_string(settings_a11y, "reduced-motion", "reduce");
        } else {
            g_settings_set_string(
                settings_a11y, "reduced-motion", "no-preference"
            );
        }
    } else if (g_strcmp0(key, "contrast") == 0) {
        if (g_strcmp0(value, "high") == 0) {
            g_settings_set_boolean(settings_a11y, "high-contrast", TRUE);
        } else {
            g_settings_set_boolean(settings_a11y, "high-contrast", FALSE);
        }
    }

    g_free(key);
    g_free(value);
}

static void on_monitor_changed(
    GFileMonitor *monitor,
    GFile *file,
    GFile *other_file,
    GFileMonitorEvent event_type,
    gpointer user_data
) {
    if (event_type == G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT) {
        on_change(file);
    }
}

static void scan_initial_files(GFile *dir) {
    GError *error = NULL;
    GFileEnumerator *enumerator = g_file_enumerate_children(
        dir,
        G_FILE_ATTRIBUTE_STANDARD_NAME,
        G_FILE_QUERY_INFO_NONE,
        NULL,
        &error
    );

    if (error) {
        g_error("Failed to enumerate directory: %s", error->message);
        exit(EXIT_FAILURE);
    }

    GFileInfo *info;
    GFile *child;
    while ((info = g_file_enumerator_next_file(enumerator, NULL, NULL))) {
        child = g_file_enumerator_get_child(enumerator, info);
        on_change(child);
        g_object_unref(child);
        g_object_unref(info);
    }

    g_object_unref(enumerator);
}

int main(void) {
    settings = g_settings_new("org.gnome.desktop.interface");
    settings_a11y = g_settings_new("org.gnome.desktop.a11y.interface");

    gchar *root = g_build_filename(
        g_get_user_config_dir(), "org.freedesktop.appearance", NULL
    );
    GFile *dir = g_file_new_for_path(root);
    g_free(root);

    GError *error = NULL;
    GFileMonitor *monitor =
        g_file_monitor_directory(dir, G_FILE_MONITOR_NONE, NULL, &error);
    if (error) {
        g_error("Failed to create monitor: %s", error->message);
        return EXIT_FAILURE;
    }
    g_signal_connect(monitor, "changed", G_CALLBACK(on_monitor_changed), NULL);

    scan_initial_files(dir);

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    g_main_loop_unref(loop);
    g_object_unref(monitor);
    g_object_unref(dir);
    g_object_unref(settings);
    g_object_unref(settings_a11y);

    return EXIT_SUCCESS;
}
