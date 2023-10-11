#include<stdio.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<unistd.h>
#include<string.h>

void error(const char *msg){
    perror(msg);
    exit(1);
}

int main(){
    return 0;
}