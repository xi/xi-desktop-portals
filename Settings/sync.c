#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <gio/gio.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

char root[PATH_MAX - 64];
GSettings *settings, *settings_a11y;

void on_change(const char *key) {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/%s", root, key);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return;
    }
    char value[256];
    int len = read(fd, value, sizeof(value) - 1);
    close(fd);
    if (len < 0) {
        return;
    }
    value[len] = '\0';
    g_strchomp(value);

    if (strcmp(key, "color-scheme") == 0) {
        if (strcmp(value, "dark") == 0) {
            g_settings_set_string(settings, "color-scheme", "prefer-dark");
        } else if (strcmp(value, "light") == 0) {
            g_settings_set_string(settings, "color-scheme", "prefer-light");
        } else {
            g_settings_set_string(settings, "color-scheme", "default");
        }
    } else if (strcmp(key, "accent-color") == 0) {
        // TODO: translate color to color name
        g_settings_set_string(settings, "accent-color", value);
    } else if (strcmp(key, "reduced-motion") == 0) {
        if (strcmp(value, "reduced") == 0) {
            g_settings_set_string(settings_a11y, "reduced-motion", "reduce");
        } else {
            g_settings_set_string(settings_a11y, "reduced-motion", "no-preference");
        }
    } else if (strcmp(key, "contrast") == 0) {
        if (strcmp(value, "high") == 0) {
            g_settings_set_boolean(settings_a11y, "high-contrast", TRUE);
        } else {
            g_settings_set_boolean(settings_a11y, "high-contrast", FALSE);
        }
    }
}

int main() {
    snprintf(
        root, sizeof(root), "%s/org.freedesktop.appearance", g_get_user_config_dir()
    );

    settings = g_settings_new("org.gnome.desktop.interface");
    settings_a11y = g_settings_new("org.gnome.desktop.a11y.interface");

    int fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
        return EXIT_FAILURE;
    }
    int wd = inotify_add_watch(fd, root, IN_CLOSE_WRITE);
    if (wd < 0) {
        perror("inotify_add_watch");
        return EXIT_FAILURE;
    }

    DIR *d = opendir(root);
    struct dirent *entry;
    if (d) {
        while ((entry = readdir(d))) {
            on_change(entry->d_name);
        }
        closedir(d);
    }

    int epfd = epoll_create(1);
    struct epoll_event ev, events[1];
    ev.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);

    char buf[BUF_LEN];
    struct inotify_event *event;

    while (1) {
        epoll_wait(epfd, events, 1, -1);

        int length = read(fd, buf, sizeof(buf));
        if (length < 0) {
            perror("read");
            return EXIT_FAILURE;
        }

        int i = 0;
        while (i < length) {
            event = (struct inotify_event *)&buf[i];
            on_change(event->name);
            i += EVENT_SIZE + event->len;
        }
    }
}
