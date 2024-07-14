#include <stdio.h>
#include <winsock2.h>
#include <Windows.h>

#define BUFFER_SIZE 1024


// Function to get the content type based on file extension
const char* get_content_type(const char* path) {
    const char* ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".jpg") == 0) return "image/jpeg";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    return "application/octet-stream";
}

void send_response(SOCKET client, const char* status, const char* content_type, const char* content) {
    char buffer[BUFFER_SIZE];
    int content_length = strlen(content);
    snprintf(buffer, BUFFER_SIZE,
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "\r\n"
        "%s",
        status, content_type, content_length, content);
    send(client, buffer, strlen(buffer), 0);
}

void handle_client(SOCKET client) {
    char request[BUFFER_SIZE] = {0};
    int bytes_received = recv(client, request, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        closesocket(client);
        return;
    }

    printf("Received request:\n%s\n", request);

    // Parse the request line
    char method[16], path[256], protocol[16];
    sscanf(request, "%s %s %s", method, path, protocol);

    printf("Parsed request:\nMethod: %s\nPath: %s\nProtocol: %s\n", method, path, protocol);

    if (strcmp(method, "GET") == 0) {
        char* file_name = path + 1;  // Skip the leading '/'
        if (strcmp(file_name, "") == 0) {
            file_name = "index.html";
        }

        FILE* file = fopen(file_name, "rb");
        if (file == NULL) {
            perror("fopen");
            send_response(client, "404 Not Found", "text/plain", "404 Not Found");
        } else {
            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            fseek(file, 0, SEEK_SET);
            char* file_content = malloc(file_size + 1);
            fread(file_content, 1, file_size, file);
            file_content[file_size] = '\0';
            fclose(file);

            const char* content_type = get_content_type(file_name);
            send_response(client, "200 OK", content_type, file_content);
            free(file_content);
        }
    } else {
        send_response(client, "400 Bad Request", "text/plain", "400 Bad Request");
    }

    closesocket(client);
}

int main(void) {
    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        return 1;
    }

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed.\n");
        WSACleanup();
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);
    if (bind(server_socket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed.\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, 10) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed.\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Server listening on port 8080...\n");

    while (1) {
        SOCKET client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            fprintf(stderr, "Accept failed.\n");
            continue;
        }

        // Handle client request in a separate thread (simple approach, not optimal for high load)
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handle_client, (LPVOID)client_socket, 0, NULL);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}