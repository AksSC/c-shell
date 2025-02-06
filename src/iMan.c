#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netdb.h>
#include<unistd.h>
#include "iMan.h"
#include "command.h"

void remove_tags(char *text){
    int in_tag = 0;
    char *src = text;
    char *dst = text;

    while(*src){
        if(*src == '<'){
            in_tag = 1;
        }else if(*src == '>'){
            in_tag = 0;
        }else if(!in_tag){
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}

void iMan(char *command){
    int sockfd;
    struct addrinfo hints, *res;
    char buffer[LEN];
    char request[LEN];
    int bytes_received;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;      // forve version 4 or 6
    hints.ai_socktype = SOCK_STREAM;  

    if(getaddrinfo("man.he.net", "80", &hints, &res) != 0) {
        perror("getaddrinfo");
        exit(1);
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(sockfd == -1){
        perror("socket");
        exit(1);
    }

    // Connect
    if(connect(sockfd, res->ai_addr, res->ai_addrlen) == -1){
        perror("connect");
        close(sockfd);
        exit(1);
    }

    freeaddrinfo(res);

    // GET
    snprintf(request, sizeof(request),
             "GET /man1/%s HTTP/1.0\r\n"
             "Host: man.he.net\r\n"
             "Connection: close\r\n\r\n",
             command);

    // send request
    if(send(sockfd, request, strlen(request), 0) == -1){
        perror("send");
        close(sockfd);
        exit(1);
    }

    while((bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0){
        buffer[bytes_received] = '\0';

        remove_tags(buffer); // Remove tags? What if there is some extra <> that is not a tag...? Not necessary though so should I not...?

        int newlines = 0;
        for(int i = 0; i < bytes_received; i++){
            if(buffer[i] == '\n'){
                newlines++;
            }
            if(newlines == 6){
                memmove(buffer, buffer + i + 1, bytes_received - i);
                break;
            }
        }

        printf("%s", buffer);
    }

    // In case of an erroneous command, one more error message after buffer indicating the same? Check specifications again I guess.

    if(bytes_received == -1){
        perror("recv");
    }

    close(sockfd);
}
