#include<stdio.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<unistd.h>
#include<string.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 9020

void connection_handler(int sockFD); // Handles the read & write operations to the server

void error(const char *msg){
    perror(msg);
    exit(1);
}

int main(){
    int client_socket;
    struct sockaddr_in server_addr;

    // create a socket
    //"client_socket" is socket descriptor of server
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket == -1)
        error("Error in socket creation");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT); // Port is same as specified in server
    // specified destination address of socket
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    
    
    // Connect to the server
    if(connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        error("Error in connecting to the server");
    
    char display_string[] = "client side : socket created successfully !!\n";
    write(1,display_string, sizeof(display_string));
    char display_string_2[] = "Client to server connection succesfully established !!\n";
    write(1, display_string_2, sizeof(display_string_2));
    
    connection_handler(client_socket);

    close(client_socket);
    
    return 0;
}

// Handles the read & write operations of the server
void connection_handler(int sockFD)
{
    char readBuffer[1024], writeBuffer[1024]; // A buffer used for reading from / writting to the server
    int readBytes, writeBytes;            // Number of bytes read from / written to the socket

    char tempBuffer[1024];

    do
    {
        bzero(readBuffer, sizeof(readBuffer)); // Empty the read buffer
        bzero(tempBuffer, sizeof(tempBuffer)); // Empty the write buffer
        readBytes = read(sockFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
            perror("Error while reading from client socket!");
        else if (readBytes == 0)
            printf("Closing the connection to the server now!\n");
        else if (strchr(readBuffer, '^') != NULL)
        {
            // Skip read from client
            strncpy(tempBuffer, readBuffer, strlen(readBuffer) - 1);
            printf("%s\n", tempBuffer);
            writeBytes = write(sockFD, "^", strlen("^"));
            if (writeBytes == -1)
            {
                perror("Error while writing to client socket!");
                break;
            }
        }
        else if (strchr(readBuffer, '$') != NULL)
        {
            // Server sent an error message and is now closing it's end of the connection
            strncpy(tempBuffer, readBuffer, strlen(readBuffer) - 2);
            printf("%s\n", tempBuffer);
            printf("Closing the connection to the server now!\n");
            break;
        }
        else
        {
            bzero(writeBuffer, sizeof(writeBuffer)); // Empty the write buffer

            if (strchr(readBuffer, '#') != NULL)
                strcpy(writeBuffer, getpass(readBuffer));
            else
            {
                printf("%s\n", readBuffer);
                scanf("%[^\n]%*c", writeBuffer); // Take user input!
            }

            writeBytes = write(sockFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing to client socket!");
                printf("Closing the connection to the server now!\n");
                break;
            }
        }
    } while (readBytes > 0);

    close(sockFD);
}