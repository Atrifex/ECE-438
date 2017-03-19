/*
 * MP1 http_server
 * Author: Rishi Thakkar
 * Assignment: MP1
 * Date: 2/08/2017
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <dirent.h>

// Error checking
#define ERROR -1
#define TRUE 1
#define FALSE 0
#define MAX_PORT 65535
#define MAXDATASIZE 2000
#define INIT_BUF_SIZE 2000

// Server Properties
#define BACKLOG 20

// Communication Constants
#define GET_S "GET /"
#define HTTP_0 "HTTP/1.0"
#define HTTP_1 "HTTP/1.1"
#define RESP_200 "HTTP/1.0 200 OK\r\n\r\n"
#define RESP_404 "HTTP/1.0 404 Not Found\r\n\r\nError: 404\nWhoops, file not found!\n"
#define RESP_400 "HTTP/1.0 400 Bad Request\r\n\r\nError: 400\nBad Request\n"
#define TWO_CRLF "\r\n\r\n"

typedef struct recv_buf_t{
    char * buf;
    unsigned long long size;
    unsigned long long stored;
}recv_buf_t;

/**
 Function checks errors in the input port string.
 Makes sure that it it is a number between 0 and MAX_PORT

 @param port - string containing input port
 @return - error val
 */
int error_check_inputs(char * port)
{
    int port_val = atoi(port);
    char * port_s = (char *) malloc((strlen(port)+1)*sizeof(char));
    memset(port_s, '\0', (strlen(port)+1)*sizeof(char));
    sprintf(port_s, "%d", port_val);
    if(((port_val == 0) && (strcmp(port, "0") != 0)) || (strlen(port_s) != strlen(port))
       || (port_val > MAX_PORT) || (port_val < 0)){
        free(port_s);
        return ERROR;
    }
    
    free(port_s);
    return 0;
}


/**
 Sets up the server on the specified port through the use of system libs.

 @param port - port to start listening for
 @return - return sockfd
 */
int server_setup(char * port)
{
    int sockfd;
    int optval = 1;
    int gai_rv;
    struct addrinfo hints, *servinfo, *p;

    // setup structs
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;        // use my IP
    
    if ((gai_rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai_rv));
        return 1;
    }
    
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        
        if (setsockopt(sockfd, SOL_SOCKET, (SO_REUSEADDR | SO_REUSEPORT), &optval,
                       sizeof(optval)) == -1) {
            perror("setsockopt");
            freeaddrinfo(servinfo); // all done with this structure
            exit(1);
        }
        
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        
        break;
    }
    
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        freeaddrinfo(servinfo); // all done with this structure
        return 2;
    }
    
    freeaddrinfo(servinfo); // all done with this structure
    
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    
    return sockfd;
}

/**
 Sends HTTP 400 error. This occurs when GET request sent is not valid.

 @param clientfd - the fd
 @return - error val
 */
int send_400_error(int clientfd)
{
    if(send(clientfd, RESP_400, strlen(RESP_400), 0) == -1){
        perror("send");
    }
    return ERROR;
}

/**
 Sends HTTP 404 error. This occurs when file is not found

 @param clientfd - the fd
 @return - error val
 */
int send_404_error(int clientfd)
{
    if(send(clientfd, RESP_404, strlen(RESP_404), 0) == -1){
        perror("send");
    }
    return ERROR;
}

/**
 Sends file. 

 @param clientfd - the fd
 @param fileptr - the file pointer
 @return - error val
 */
int send_file(int clientfd, FILE * fileptr)
{
    char buf_out[MAXDATASIZE];
    int bytes_read;
    
    if(send(clientfd, RESP_200, strlen(RESP_200), 0) == -1){
        perror("send");
        fclose(fileptr);
        return ERROR;
    }
    
    while((bytes_read = fread(buf_out, sizeof(char), MAXDATASIZE, fileptr)) != 0) {
        if(send(clientfd, buf_out, bytes_read, 0) == -1){
            perror("send");
            fclose(fileptr);
            return ERROR;
        }
    }
    
    fclose(fileptr);
    return 0;
}

/**
 Function processes the GET request sent by the client

 @param clientfd - client fd
 @param request - request string
 @return - error val
 */
int process_request(int clientfd, char * request)
{
    FILE * file_req;
    DIR * dir_check;
    if(strncmp(request, GET_S, strlen(GET_S)) != 0 ||
       ((strstr(request, HTTP_0) == NULL) && (strstr(request, HTTP_1) == NULL))){
        return send_400_error(clientfd);
    }
    
    request+= strlen(GET_S) - 1;
    request = strtok(request, " ");
    request++;
    
    if(*request == '\0'){
        return send_400_error(clientfd);
    }
    
    file_req = fopen(request, "r");
    dir_check = opendir(request);

    // Check if request is for directory
    if(dir_check != NULL){
        closedir(dir_check);
        fclose(file_req);
        return send_404_error(clientfd);
    }

    // if not then check file status
    if(file_req == NULL){
        return send_404_error(clientfd);
    } else{
        return send_file(clientfd, file_req);
    }
}

/**
 Function executed by the client handler thread.

 @param clientfdptr - Pointer to a malloced value of clientfd
 @return - return error as a pointer (Not used for anything)
 */
void * connection_handler(void * clientfdptr)
{
    recv_buf_t buffer;
    int bytes_read;

    buffer.buf = (char *) malloc(INIT_BUF_SIZE*sizeof(char));
    buffer.size = INIT_BUF_SIZE;
    buffer.stored = 0;
    
    int clientfd = *((int *)clientfdptr);
    
    // Get the GET request
    do{
        if ((bytes_read = recv(clientfd, buffer.buf + buffer.stored, (buffer.size/2), 0)) 
            == -1) {
            perror("recv");
            close(clientfd);
            free(buffer.buf);
            free(clientfdptr);
            return (void * )1;
        }

        buffer.stored += bytes_read;
        if(buffer.stored > (buffer.size/2)){
            buffer.buf = (char *)realloc(buffer.buf, 2*buffer.size);
            buffer.size *= 2;
        }
        buffer.buf[buffer.stored] = '\0';

        if(strstr(buffer.buf, TWO_CRLF) != NULL)
            break;
    }while(TRUE);

    buffer.buf[buffer.stored] = '\0';

    // Process the GET request
    if(process_request(clientfd, buffer.buf) == ERROR){
        close(clientfd);
        free(buffer.buf);
        free(clientfdptr);
        return (void * )1;
    }
    
    close(clientfd);
    free(buffer.buf);
    free(clientfdptr);
    return (void *)0;
}

/**
 Waits for connections to come in and dispatches them to threads.
 The threads look at the requests and handle them.

 @param sockfd - the fd of port on which server is listening
 */
void wait_for_connections(int sockfd)
{
    int clientfd, * clientfdptr;    // new connection on clientfd
    struct sockaddr_storage client_addr; // connector's address information
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    pthread_t thread;
    pthread_attr_t attr;
    
    // init pthreads attributes
    pthread_attr_init(&attr);
    
    while(1) {  // main accept() loop
        sin_size = sizeof(client_addr);
        if((clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size)) == -1){
            perror("accept");
            continue;
        }
        
        // store client fd dynamically
        clientfdptr = (int *)malloc(sizeof(int));
        *clientfdptr = clientfd;

        // launch thread
        pthread_create(&thread, &attr, connection_handler, (void *) clientfdptr);
    }

}



int main(int argc, char *argv[])
{
    int sockfd;
    
    // Erorr Checking of Inputs
    if((argc != 2) || (error_check_inputs(argv[1]) == ERROR)){
        fprintf(stderr,"usage: ./http_server port\nPort Range: 0 - 65535");
        exit(1);
    }
    
    // Setup server
    sockfd = server_setup(argv[1]);
    
    // Wait for connections
    wait_for_connections(sockfd);
    
    return 0;
}

