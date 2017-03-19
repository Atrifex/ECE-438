/*
 * MP1 http_client
 * Author: Rishi Thakkar
 * Assignment: MP1
 * Date: 2/07/2017
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

// Error checking
#define PREFIX_S "http://"
#define ERROR -1
#define TRUE 1
#define FALSE 0

// Default values
#define DEFAULT_PORT "80"
#define MAXDATASIZE 3000  // max bytes for communication

// Communication Values
#define GET_S "GET /"
#define HTTP_S " HTTP/1.0\r\n"
#define HOST_S "Host: "
#define CRLF "\r\n"
#define TWO_CRLF "\r\n\r\n"
#define RDIR_RESP_1 "HTTP/1.1 301"
#define RDIR_RESP_0 "HTTP/1.0 301"
#define LOCATION "Location: "
#define REDIR 1
#define COMM_SUCCESS 0
#define DEFAULT 0



/**
 A modified version of strtok_mod that returns a poitner to the second
 half of the token instead of the first

 @param str - Input string to be broken into tokens
 @param delim - delimitor to use to break
 @return - pointer to the second toke
 */
char * strtok_mod(char *str, const char *delim)
{
    int lengthPre, lengthPost;

    lengthPre = strlen(str);
    strtok(str, delim);
    lengthPost = strlen(str);

    return (lengthPre == lengthPost)? NULL : (str + lengthPost + 1);
}


/**
 Used to parse the inputs into the 3 chunks of information that are needed
 to connect to the server.

 @param info - an array of character pointers in which to store parsed infromation
 @param arguments - string to parse
 @return - error value
 */
int parse_inputs(char * info[4], char * arguments)
{
    if(strncmp(arguments, PREFIX_S, strlen(PREFIX_S)) != 0)
        return ERROR;

    arguments[0] = '8';
    arguments[1] = '0';
    arguments[2] = '\0';

    // extract server name
    info[0] = arguments + 7;

    // extract path
    info[2] = strtok_mod(info[0], "/");
    if(info[2] == NULL){
        info[2] = arguments + 2;
    }

    // extract port or use default port
    info[1] = strtok_mod(info[0], ":");
    if(info[1] == NULL){
        info[1] = arguments;
    }

    return 0;
}

/**
 Parses the url provided by the HTTP response for redirection.
 It uses parse_inputs() as a helper function.

 @param sockfd - the fd
 @param info - array of http communication info
 @param redirectionURL - the URL to redirect to
 */
void parse_redirection(int sockfd, char * info[4], char * redirectionURL)
{
    unsigned int url_length;
    char * parser;

    // point to url
    redirectionURL += strlen(LOCATION);
    parser = strchr(redirectionURL, '\r');
    url_length = parser - redirectionURL;
    info[3] = (char *)malloc((url_length+1)*sizeof(char));
    strncpy(info[3], redirectionURL, url_length);
    info[3][url_length] = '\0';

    if(parse_inputs(info, info[3]) == ERROR){
        close(sockfd);
        fprintf(stderr,"Redirection URL not valid\n");
        exit(1);
    }
}

/**
 Function creates a TCP connection with a server

 @param server - string with hostname
 @param port - port to connect to
 @return - sockfd
 */
int connect_TCP(char * server, char * port)
{
    int sockfd;
    struct addrinfo hints, * servinfo, * p;
    int gai_rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((gai_rv = getaddrinfo(server, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai_rv));
        exit(1);
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        // get a socket to be able to communicate
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        // connect with server
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    // if the addresses are all invalid
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        freeaddrinfo(servinfo); // all done with this structure
        exit(2);
    }

    freeaddrinfo(servinfo); // all done with this structure

    return sockfd;
}


/**
 Abstraction of the get request. User only needs to pass the path.
 Everyting else is done using default values.


 @param sockfd - the fd
 @param host - hostname
 @param buffer - path of file
 */
void get_request(int sockfd, char * host, char * port, char * path)
{
    int bytes_sent;
    if ((bytes_sent = send(sockfd, GET_S, strlen(GET_S), 0)) == -1){
        perror("send");
        close(sockfd);
        exit(1);
    }
    if ((bytes_sent = send(sockfd, path, strlen(path), 0)) == -1){
        perror("send");
        close(sockfd);
        exit(1);
    }
    if ((bytes_sent = send(sockfd, HTTP_S, strlen(HTTP_S), 0)) == -1){
        perror("send");
        close(sockfd);
        exit(1);
    }
    if ((bytes_sent = send(sockfd, HOST_S, strlen(HOST_S), 0)) == -1){
        perror("send");
        close(sockfd);
        exit(1);
    }
    if ((bytes_sent = send(sockfd, host, strlen(host), 0)) == -1){
        perror("send");
        close(sockfd);
        exit(1);
    }
    if ((bytes_sent = send(sockfd, ":", 1, 0)) == -1){
        perror("send");
        close(sockfd);
        exit(1);
    }
    if ((bytes_sent = send(sockfd, port, strlen(port), 0)) == -1){
        perror("send");
        close(sockfd);
        exit(1);
    }
    if ((bytes_sent = send(sockfd, TWO_CRLF, strlen(TWO_CRLF), 0)) == -1){
        perror("send");
        close(sockfd);
        exit(1);
    }
}

/**
 Function is used to receive a HTTP response and check for redirection.
 If there is redirection then it handles it correctly and tells main that a new connection
 needs to be formed.

 @param sockfd - the fd
 @param info - array of http communication info
 @return - returns redirection status
 */
int recv_msg(int sockfd, char * info[4])
{
    FILE * outputFile;
    int bytes_read;
    char buf_in[MAXDATASIZE];
    char * message_parser, * str_redir;
    char parse_for_msgbdy = TRUE;
    static unsigned char redir_flag = DEFAULT;


    // Open file for storing messages
    outputFile = fopen("output", "w");

    // free pointers to prepare for redirection
    if(redir_flag == REDIR){
        // NOTE: done only after being redirected once
        free(info[3]);
        info[3] = NULL;
    }

    do{
        // read MAXDATASIZE characters
        if ((bytes_read = recv(sockfd, buf_in, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            close(sockfd);
            exit(1);
        }
        buf_in[bytes_read] = '\0';

        // parse to find message body
        if(parse_for_msgbdy == TRUE){
            // check for redirection
            if((strstr(buf_in, RDIR_RESP_0) != NULL) || (strstr(buf_in, RDIR_RESP_1) != NULL)){
                if((str_redir = strstr(buf_in, LOCATION)) != NULL){
                    parse_redirection(sockfd, info, str_redir);
                    redir_flag = REDIR;
                    close(sockfd);
                    return REDIR;
                }
            }
            // search for end of header
            if((message_parser = strstr(buf_in, TWO_CRLF)) == NULL)
                continue;

            // write message to output file
            message_parser += strlen(TWO_CRLF);
            fwrite(message_parser, sizeof(char), bytes_read - (message_parser - buf_in), outputFile);
            parse_for_msgbdy = FALSE;

        } else{
            // write message to output file
            fwrite(buf_in, sizeof(char), bytes_read, outputFile);
        }
        fflush(outputFile);
    } while(bytes_read != 0);

    // close the file being written to
    fclose(outputFile);

    return COMM_SUCCESS;
}

int main(int argc, char *argv[])
{
    int sockfd;
    char * http_conn_info[4];
    unsigned char redir_flag = DEFAULT;

    // Error Checking and Parsing inputs
    if ((argc != 2) || (parse_inputs(http_conn_info, argv[1]) == ERROR)){
        fprintf(stderr,"usage: ./http_client http://hostname[:port]/path/to/file\n");
        exit(1);
    }

    // Connect and Communicate
    do {
        // connect to server
        sockfd = connect_TCP(http_conn_info[0], http_conn_info[1]);

        // send GET Request
        get_request(sockfd, http_conn_info[0], http_conn_info[1], http_conn_info[2]);

        // read message or retry if redirected
    } while(recv_msg(sockfd, http_conn_info) == REDIR);

    return 0;
}
