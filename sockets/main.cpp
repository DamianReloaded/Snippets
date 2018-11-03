#include <iostream>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void * server(void * ptr)
{
    int sockfd, newsockfd, portno;
    unsigned int clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");
    bzero( (char *) &serv_addr, sizeof(serv_addr)); //memset
    portno = atoi("1337");
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) <0) error("ERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) error ("ERROR on accept");
    bzero(buffer,256);
    n = read(newsockfd, buffer, 255);
    if (n < 0) error ("ERROR reading from socket");
    printf("Here is the message: %s\n", buffer);
    n = write(newsockfd,"I got your message", 18);
    if (n<0) error ("ERROR writing to socket");
    return NULL;
}

void * client(void * ptr)
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent * server;
    char buffer[256];
    portno = atoi("1337");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd<0) error("ERROR opening socket");
    server = gethostbyname("127.0.0.1");
    if (server == NULL) error ("ERROR no such host");
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) error("ERROR connecting");
    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n<0) error("ERROR writing to socket");
    bzero(buffer, 256);
    n = read(sockfd,buffer,255);
    if (n < 0) error ("ERROR reading from socket");
    printf("%s\n",buffer);
    return NULL;
}

#include <pthread.h>

int main()
{
    pthread_t thread_client;
    if (pthread_create(&thread_client, NULL, client, NULL))
    {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    server(NULL);

    if (pthread_join(thread_client,NULL))
    {
        fprintf(stderr, "Error joining thread\n");
        return 2;
    }

    return 0;
}
