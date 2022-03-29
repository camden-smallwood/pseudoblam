#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

int main()
{
    // --------------------------------------------------------------------------------
    // resolve the server url
    // --------------------------------------------------------------------------------

    const char *server_url = "mainnet.infura.io";

    struct hostent *server_host = gethostbyname(server_url);
    
    if (!server_host)
    {
        perror("ERROR: No such host");
        exit(EXIT_FAILURE);
    }

    if (!server_host->h_addr_list[0])
    {
        perror("ERROR: No address associated with host");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(80);

    memcpy(
        &server_address.sin_addr.s_addr,
        server_host->h_addr_list[0],
        server_host->h_length);

    // --------------------------------------------------------------------------------
    // create the client socket
    // --------------------------------------------------------------------------------

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket == -1)
    {
        perror("ERROR: Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // --------------------------------------------------------------------------------
    // connect to the server
    // --------------------------------------------------------------------------------

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("ERROR: Failed to connect to host");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    puts("Connected to server");

    // --------------------------------------------------------------------------------
    // send a message to the server
    // --------------------------------------------------------------------------------

    char *message = NULL;
    asprintf(
        &message,
        "GET /mychat HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        // "Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n"
        "Sec-WebSocket-Protocol: chat\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "Origin: http://%s\r\n",
        server_url,
        server_url);
    
    int message_length = strlen(message);

    for (int current_sent = 0, total_sent = 0;
        total_sent < message_length;
        total_sent += current_sent)
    {
        current_sent = write(client_socket, message, message_length - total_sent);

        if (current_sent < 0)
        {
            perror("ERROR: Failed to send client data to server");
            free(message);
            close(client_socket);
            exit(EXIT_FAILURE);
        }
        else if (current_sent == 0)
        {
            break;
        }
    }

    printf("Sent message to server: %s\n", message);
    
    free(message);

    // --------------------------------------------------------------------------------
    // read the response from the server
    // --------------------------------------------------------------------------------

    char *response = NULL;

    for (;;)
    {
        puts("Attempting to read from server...");

        char response_buffer[1024];
        memset(response_buffer, 0, sizeof(response_buffer));

        int current_read = read(client_socket, response_buffer, sizeof(response_buffer) - 1);

        puts("Finished reading from server");

        if (current_read < 0)
        {
            perror("ERROR: Failed to read client response from server");
            free(response);
            close(client_socket);
            exit(EXIT_FAILURE);
        }
        else if (current_read == 0)
        {
            break;
        }
        else
        {
            response_buffer[current_read] = '\0';
            printf("Current response buffer: %s\n", response_buffer);
        }

        asprintf(&response, "%s%s", response ? response : "", response_buffer);
    }

    if (!response)
    {
        perror("ERROR: No client response received from server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Received response from server: %s\n", response);

    free(response);

    // --------------------------------------------------------------------------------
    // close connections
    // --------------------------------------------------------------------------------

    close(client_socket);

    puts("Done");

    return 0;
}
