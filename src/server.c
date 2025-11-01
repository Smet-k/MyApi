#include "server.h"

#include <ctype.h>
#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tpool.h"
#include "request.h"
#include "api_response.h"
#define MAX_CLIENTS 1024
#define BUFFER_SIZE 2048
#define SERVERSTRING "Server: customApi\r\n"
#define ROOTFOLDER "html"

typedef struct {
    int client_sock;
    Config* cfg;
} request_context_t;

static void startup(int* server_fd, struct sockaddr_in* server_addr, Config* cfg);
static void accept_requests(int server_fd, Config cfg);

static void process_requests(void* arg);

static void respond(int client, int code, const char* reason, const char* body);
// static void respond_file(int client, http_request_t request, FILE* resource);

static void handle_request(int client, http_request_t request, Config* cfg);

void run_server(Config cfg) {
    struct sockaddr_in server_addr;
    int server_sock;

    startup(&server_sock, &server_addr, &cfg);

    accept_requests(server_sock, cfg);

    close(server_sock);
}

static void startup(int* server_fd, struct sockaddr_in* server_addr, Config* cfg) {
    if ((*server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Server start failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = INADDR_ANY;

    while (1) {
        server_addr->sin_port = htons(cfg->port);

        if (bind(*server_fd,
                 (struct sockaddr*)server_addr,
                 sizeof(*server_addr)) == 0) {
            break;
        }

        if (errno == EADDRINUSE)
            cfg->port++;
        else {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }

        if (cfg->port > 65535) {
            fprintf(stderr, "No free ports available.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (listen(*server_fd, 10) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Listening at localhost:%u\n", cfg->port);
}

static void accept_requests(int server_fd, Config cfg) {
    ThreadPool* tp = threadpool_create(cfg.threads);
    struct sockaddr_in client_addr;
    socklen_t client_addr_length = sizeof(client_addr);

    struct pollfd fds[MAX_CLIENTS];
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;
    nfds_t nfds = 1;

    while (1) {
        int ready = poll(fds, nfds, -1);
        if (ready < 0) {
            perror("poll failed");
            continue;
        }

        for (int i = 0; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == server_fd) {
                    int client_sock = accept(server_fd,
                                             (struct sockaddr*)&client_addr,
                                             &client_addr_length);

                    if (client_sock >= 0) {
                        fds[nfds].fd = client_sock;
                        fds[nfds].events = POLLIN;
                        nfds++;
                    }
                } else {
                    // int client_sock = fds[i].fd;

                    request_context_t* ctx = malloc(sizeof(request_context_t));
                    ctx->client_sock = fds[i].fd;
                    ctx->cfg = &cfg;

                    Task task = {
                        .function = &process_requests,
                        .arg = ctx,
                    };

                    thread_queue_push(&tp->queue, task);

                    fds[i] = fds[nfds - 1];
                    nfds--;
                }
            }
        }
    }
}

static void process_requests(void* arg) {
    request_context_t* ctx = (request_context_t*)arg;
    int client = ctx->client_sock;
    Config* cfg = ctx->cfg;

    // free(ctx);

    char buf[BUFFER_SIZE];
    http_request_t request;

    int numbytes = recv(client, buf, sizeof(buf) - 1, 0);
    if (numbytes <= 0) {
        close(client);
        return;
    }

    buf[numbytes] = '\0';
    request = parse_request(buf);

    if (request.content_length < 0 && request.request_line.method == HTTP_POST) {
        respond(client, 400, "Bad Request", "");
        return;
    }

    switch (request.request_line.method)
    {
    case HTTP_GET:
    case HTTP_POST:
        handle_request(client, request, cfg);
        break;
    default:
        respond(client, 501, "Method Not Implemented", "");
        break;
    }
    free(arg);
    close(client);
}

static void respond(int client, int code, const char* reason, const char* body) {
    char buf[BUFFER_SIZE];

    int body_len = body ? strlen(body) : 0;

    sprintf(buf, "HTTP/1.0 %d %s ", code, reason);
    send(client, buf, strlen(buf), 0);

    strcpy(buf, SERVERSTRING);
    send(client, buf, strlen(buf), 0);

    sprintf(buf, "Content-Length: %d\r\n" , body_len);
    send(client, buf, strlen(buf), 0);

    strcpy(buf, "Content-Type: application/json\r\n");
    send(client, buf, strlen(buf), 0);

    strcpy(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

    if (body && body_len > 0){
        send(client, body, body_len, 0);
    }

}

// static void respond_file(int client, http_request_t request, FILE* resource){
//     char buf[BUFFER_SIZE];
//     size_t bytes_read;
//
//     sprintf(buf, "HTTP/1.0 200 OK");
//     send(client, buf, strlen(buf), 0);
//
//     strcpy(buf, SERVERSTRING);
//     send(client, buf, strlen(buf), 0);
//
//     if (request.body) {
//         sprintf(buf, "Content-Length: %d", request.content_length);
//         send(client, buf, strlen(buf), 0);
//     }
//
//     sprintf(buf, "Content-Type: %s\r\n", get_mime_type(request.request_line.path));
//     send(client, buf, strlen(buf), 0);
//
//     strcpy(buf, "\r\n");
//     send(client, buf, strlen(buf), 0);
//
//     while ((bytes_read = fread(buf, 1, sizeof(buf) - 1, resource)) > 0) {
//         buf[bytes_read] = '\0';
//         if (request.body) {
//             char* body_end = strstr(buf, "</body>");
//             if (body_end) {
//                 *body_end = '\0';
//                 send(client, buf, strlen(buf), 0);
//                 send(client, request.body, strlen(request.body), 0);
//                 send(client, "</body>", 7, 0);
//             }
//         } else {
//             send(client, buf, bytes_read, 0);
//         }
//     }
// }


static void handle_request(int client, http_request_t request, Config* cfg) {
    api_request_t* request_params = malloc(sizeof(api_request_t));
    char param[64] = {0};

    const char *path = request.request_line.path;
    char *query = strchr(path, '?');

    if (query) {
        *query='\0';
        query++;
        request_params->query = query;
    } else {request_params->query = NULL;}
    route_t* route = lookup_route(cfg->router, request.request_line.path, request.request_line.method, param);
    request_params->params = param[0] ? param: NULL;

    if (route){
        api_response_t* response = route->func(request_params);
        respond(client, response->code, "OK", response->body);
    }
    else {
        respond(client, 404, "Not Found", "");
    }

    free(request_params);
}

