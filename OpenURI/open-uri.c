#include <stdio.h>
#include <sys/socket.h>
#include <gio/gio.h>

#define MAX_URI_LENGTH 1024
#define SOCK 3

int main() {
    char uri[MAX_URI_LENGTH];

    int n = recv(SOCK, &uri, sizeof(uri) - 1, 0);
    if (n < 0) {
        perror("recv");
        return EXIT_FAILURE;
    }
    uri[n] = '\0';
    g_strchomp(uri);

    GError *error = NULL;
    if (!g_app_info_launch_default_for_uri(uri, NULL, &error)) {
        send(SOCK, error->message, strlen(error->message), 0);
        send(SOCK, "\n", 1, 0);
        g_clear_error(&error);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
