#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT "1234" 

int main(void)
{
    int server;
    int new_fd;
    struct sockaddr_storage adding_addr; 
    socklen_t addrlen;
    struct addrinfo hints, *servinfo;

    char buf[256];   
    int number_bytes;

    int max_fd;        // максимальное количество файловых дескрипторов
    fd_set descriptor_list;    // список дескрипторов основного файла
    fd_set help_fds;  // список дескрипторов временного файла для select ()

    FD_ZERO(&descriptor_list);  
    FD_ZERO(&help_fds);  
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) 
    {
        perror("getaddrinfo");
        exit(1);
    }
    server = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (server < 0) 
    { 
        perror("socket");
        exit(6);
    } 
    int yes=1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (bind(server, servinfo->ai_addr, servinfo->ai_addrlen) < 0) 
    {
        perror("bind");
        exit(5);
    }
    if (listen(server, 30) == -1) // listenableto?
    {
        perror("listen");
        exit(3);
    }  
    FD_SET(server, &descriptor_list);
    max_fd = server; 

    for(;;)
    {
        help_fds = descriptor_list; 
        if (select(max_fd + 1, &help_fds, NULL, NULL, NULL) == -1)
        {
            perror("select");
            exit(4);
        }
        for(int i = 0; i <= max_fd; i++)
        {
            if (FD_ISSET(i, &help_fds)) // новое входящее соединение
            { 
                if (i == server) // Новое подключение?
                {
                    
                    addrlen = sizeof adding_addr;
                    new_fd = accept(server,
                        (struct sockaddr *)&adding_addr,
                        &addrlen);

                    if (new_fd == -1)
                    {
                        perror("accept");
                    }
                    else
                    {
                        FD_SET(new_fd, &descriptor_list); 
                        if (new_fd > max_fd)
                        {   
                            max_fd = new_fd;
                        }
                        printf("server: new connection on socket %d\n", new_fd);
                    }
                } 
                else // Обработка данных от пользователя
                {
                    
                    if ((number_bytes = recv(i, buf, sizeof buf, 0)) <= 0) // Проверка на проблемы в соединении
                    {   
                        if (number_bytes == 0)
                        { 
                            printf("server: socket %d dissconected\n", i);
                        }
                        else
                        {
                            perror("recv");
                        }
                        close(i);
                        FD_CLR(i, &descriptor_list);
                    }
                    else // Сообщение для всех кроме server и самого себя
                    {
                        for(int j = 0; j <= max_fd; j++)
                        {
                            if (FD_ISSET(j, &descriptor_list)) 
                            {
                                if (j != server && j != i)
                                {
                                    if (send(j, buf, number_bytes, 0) == -1)
                                    {
                                        perror("send");
                                    }
                                }
                            }
                        }
                    }
                }
            } 
        }
    }
    
    return 0;
}
