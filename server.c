#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>

#define BUF_SIZE 4096
#define RESPONSE_OK "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n"
#define RESPONSE_NOT_FOUND "HTTP/1.0 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 Not Found</h1>"

char *root_dir;

void *client_handler(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);

    char req_buf[BUF_SIZE];
    int bytes_received = read(client_sock, req_buf, sizeof(req_buf) - 1);
    if (bytes_received <= 0) {
        close(client_sock);
        pthread_exit(NULL);
    }
    req_buf[bytes_received] = '\0';

    if (strncmp(req_buf, "GET ", 4) != 0) {
        close(client_sock);
        pthread_exit(NULL);
    }

    char file_requested[BUF_SIZE];
    sscanf(req_buf + 4, "%s", file_requested);

    if (strcmp(file_requested, "/") == 0) {
        strcpy(file_requested, "/index.html");
    }

    if (strstr(file_requested, "..")) {
        write(client_sock, RESPONSE_NOT_FOUND, strlen(RESPONSE_NOT_FOUND));
        close(client_sock);
        pthread_exit(NULL);
    }

    char full_file_path[BUF_SIZE];
    snprintf(full_file_path, sizeof(full_file_path), "%s%s", root_dir, file_requested);

    int file_desc = open(full_file_path, O_RDONLY);
    if (file_desc < 0) {
        write(client_sock, RESPONSE_NOT_FOUND, strlen(RESPONSE_NOT_FOUND));
        close(client_sock);
        pthread_exit(NULL);
    }

    write(client_sock, RESPONSE_OK, strlen(RESPONSE_OK));

    char data_buf[BUF_SIZE];
    int n;
    while ((n = read(file_desc, data_buf, sizeof(data_buf))) > 0) {
        write(client_sock, data_buf, n);
    }

    close(file_desc);
    close(client_sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <directory>\n", argv[0]);
        exit(1);
    }

    int port_num = atoi(argv[1]);
    root_dir = argv[2];

    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_num);

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
        perror("bind");
        close(serv_sock);
        exit(1);
    }

    if (listen(serv_sock, 10) != 0) {
        perror("listen");
        close(serv_sock);
        exit(1);
    }

    printf("Web server running on port %d\n", port_num);

    while (1) {
        int *client_conn = malloc(sizeof(int));
        if (!client_conn) {
            perror("malloc");
            continue;
        }

        *client_conn = accept(serv_sock, NULL, NULL);
        if (*client_conn < 0) {
            perror("accept");
            free(client_conn);
            continue;
        }

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_handler, client_conn);
        pthread_detach(thread_id);
    }

    close(serv_sock);
    return 0;
}
