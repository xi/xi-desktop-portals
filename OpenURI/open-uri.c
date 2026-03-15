#define _GNU_SOURCE
#include <linux/openat2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/pidfd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <gio/gio.h>

#define MAX_URI_LENGTH 1024
#define SOCK 3

int get_pidfd(int sock) {
    int pidfd;
    socklen_t pidfd_len = sizeof(pidfd);
    if (getsockopt(sock, SOL_SOCKET, SO_PEERPIDFD, &pidfd, &pidfd_len) < 0) {
        perror("getsockopt");
        return -1;
    }
    return pidfd;
}

char *path_to_uri(char *path) {
    GFile *file = g_file_new_for_path(path);
    char *uri = g_file_get_uri(file);
    g_object_unref(file);
    return uri;
}

char *uri_to_path(char *uri) {
    GFile *file = g_file_new_for_uri(uri);
    char *path = g_file_get_path(file);
    g_object_unref(file);
    return path;
}

int is_readable(int fd) {
    fd_set fds;
    struct timeval timeout;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    return select(fd + 1, &fds, NULL, NULL, &timeout);
}

int stat_at_root(const char *path, const char *root, struct stat *statbuf) {
    struct open_how how = {0};
    how.flags = O_RDONLY;
    how.resolve = RESOLVE_IN_ROOT;

    int root_fd = open(root, O_RDONLY | O_DIRECTORY);
    if (root_fd < 0) {
        perror("open root");
        return -1;
    }

    int fd = syscall(SYS_openat2, root_fd, path, &how, sizeof(how));
    close(root_fd);
    if (fd < 0) {
        perror("openat");
        return -1;
    }

    if (fstat(fd, statbuf) != 0) {
        perror("fstat");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int same_on_host(char *path, int pidfd) {
    char root_sender[32];
    struct stat s_sender, s_host;

    snprintf(
        root_sender, sizeof(root_sender), "/proc/%i/root/", pidfd_getpid(pidfd)
    );
    if (stat_at_root(path, root_sender, &s_sender) != 0) {
        return -1;
    }
    if (stat_at_root(path, "/", &s_host) != 0) {
        return -1;
    }
    if (s_sender.st_dev != s_host.st_dev || s_sender.st_ino != s_host.st_ino) {
        return -1;
    }
    if (is_readable(pidfd) != 0) {
        return -1;
    }

    return 0;
}

int main() {
    char uri[MAX_URI_LENGTH];

    int n = recv(SOCK, &uri, sizeof(uri) - 1, 0);
    if (n < 0) {
        perror("recv");
        return EXIT_FAILURE;
    }
    uri[n] = '\0';
    g_strchomp(uri);

    if (strncmp(uri, "/", 1) == 0) {
        char *tmp = path_to_uri(uri);
        snprintf(uri, sizeof(uri), "%s", tmp);
        free(tmp);
    }

    if (strncmp(uri, "file://", 7) == 0) {
        int pidfd = get_pidfd(SOCK);
        if (pidfd < 0) {
            return EXIT_FAILURE;
        }
        char *path = uri_to_path(uri);
        if (same_on_host(path, pidfd) != 0) {
            return EXIT_FAILURE;
        }
        free(path);
    }

    GError *error = NULL;
    if (!g_app_info_launch_default_for_uri(uri, NULL, &error)) {
        send(SOCK, error->message, strlen(error->message), 0);
        send(SOCK, "\n", 1, 0);
        g_clear_error(&error);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
