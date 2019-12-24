#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT "1234"
#define MAXIMUM_MESSAGE_SIZE 200 

void *receive_handler(void *sock_fd)
{
    int sfd = (intptr_t) sock_fd;
    char buffer[MAXIMUM_MESSAGE_SIZE];
    int number_bytes;
    
    for(;;)
    {
        number_bytes = recv(sfd, buffer, MAXIMUM_MESSAGE_SIZE - 1, 0);
        if (number_bytes == -1)
        {
            perror("recv");
            exit(1);
        }
        else
            buffer[number_bytes] = '\0';
        printf("%s", buffer);
    }
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) 
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    char message[MAXIMUM_MESSAGE_SIZE];
    char nickname[25];
    int sockfd;  
    char sBuf[MAXIMUM_MESSAGE_SIZE];
    struct addrinfo hints, *servinfo;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) 
    {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(argv[1], PORT, &hints, &servinfo) != 0) 
    {
        perror("getaddrinfo");
        exit(2);
    }
    if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,
            servinfo->ai_protocol)) == -1) 
    { 
        perror("socket");
        exit(3);
    } 
    if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) 
    {
        close(sockfd);
        perror("connect");
        exit(4);
    }

    inet_ntop(servinfo->ai_family, get_in_addr((struct sockaddr *)servinfo->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);
 
    puts("nickname:");
    memset(&nickname, sizeof(nickname), 0);
    memset(&message, sizeof(message), 0);
    fgets(nickname, sizeof nickname, stdin);

    pthread_t recv_thread;
    //new_sockfd = malloc(sizeof(int));
    //new_sockfd = sockfd;
    
    if( pthread_create(&recv_thread, NULL, receive_handler, (void*)(intptr_t) sockfd) < 0)
    {  
        perror("could not create thread");
        return 1;
    }    
    
    puts("Welcome to the general chat!!!");

    for(;;)
    {

        memset(&sBuf, sizeof(sBuf), 0);
        fgets(sBuf, MAXIMUM_MESSAGE_SIZE, stdin); 
        
        int count = 0;
        while(count < strlen(nickname))
        {
            message[count] = nickname[count];
            count++;
        }
        count--;
        message[count] = ':';
        count++;
        for(int i = 0; i < strlen(sBuf); i++)
        {
            message[count] = sBuf[i];
            count++;
        }
        message[count] = '\0';

        if(send(sockfd, message, strlen(message), 0) < 0)
        {
            perror("send");
            return 1;
        }
        memset(&sBuf, sizeof(sBuf), 0);
    }
    

    pthread_join(recv_thread , NULL);
    close(sockfd);

    return 0;
}


