#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 80       // Ushobora guhindura port 80
#define BUFFER_SIZE 4096

// Function to log requests
void log_request(const char *method, const char *url) {
    FILE *f = fopen("server.log", "a");
    if (!f) return;
    time_t now = time(NULL);
    char tstr[64];
    strftime(tstr, sizeof(tstr), "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(f, "%s %s %s\n", tstr, method, url);
    fclose(f);
}

// Function to send response
void send_response(int client_socket, const char *method, const char *url) {
    char response[BUFFER_SIZE];

    if (strcmp(method, "GET") == 0) {
        snprintf(response, sizeof(response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n\r\n"
            "<html><body><h1>Hello from QT Test Server</h1></body></html>");
    } else if (strcmp(method, "POST") == 0) {
        snprintf(response, sizeof(response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n\r\n"
            "<html><body><h1>Requested URL: %s</h1></body></html>", url);
    } else {
        close(client_socket);
        return;
    }

    write(client_socket, response, strlen(response));
}

// Minimal HTTP parser
void parse_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes = read(client_socket, buffer, sizeof(buffer)-1);
    if (bytes <= 0) return;
    buffer[bytes] = '\0';

    char method[8];
    char url[1024];

    sscanf(buffer, "%s %s", method, url);

    log_request(method, url);
    send_response(client_socket, method, url);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_socket < 0) {
            perror("accept failed");
            continue;
        }

        parse_request(client_socket);
        close(client_socket);
    }

    close(server_fd);
    return 0;
}
