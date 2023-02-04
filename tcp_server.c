#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_CONNECTIONS 5
#define BUFFER_SIZE 1024

void *handle_client(void *socket_desc)
{
    int sock = *(int*)socket_desc;
    char buffer[BUFFER_SIZE];
    int bytes_received;
    while((bytes_received = recv(sock, buffer, BUFFER_SIZE, 0)) > 0)
    {
        buffer[bytes_received] = '\0';
        printf("Received: %s\n", buffer);
        send(sock, buffer, bytes_received, 0);
    }
    close(sock);
    free(socket_desc);
    return 0;
}

int main(int argc, char *argv[])
{
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    int address_len = sizeof(client_address);
    pthread_t thread;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        printf("Could not create socket");
        return 1;
    }
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(8888);
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    {
        printf("Bind failed");
        return 1;
    }
    listen(server_socket, MAX_CONNECTIONS);
    printf("Waiting for incoming connections...\n");
    while ((client_socket = accept(server_socket, (struct sockaddr *) &client_address, (socklen_t *) &address_len)))
    {
        printf("Accepted connection from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
        int *new_sock = malloc(sizeof(int));
        *new_sock = client_socket;
        if (pthread_create(&thread, NULL, handle_client, (void*) new_sock) < 0)
        {
            printf("Could not create thread");
            return 1;
        }
    }
    if (client_socket < 0)
    {
        printf("Accept failed");
        return 1;
    }
    close(server_socket);
    return 0;
}