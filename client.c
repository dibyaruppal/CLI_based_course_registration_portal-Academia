#include<stdio.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<unistd.h>
#include<string.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8997

void error(const char *msg){
    perror(msg);
    exit(1);
}

int main(){

    return 0;
}