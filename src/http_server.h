#pragma once
#ifndef _SERVER_H
#define _SERVER_H

#ifndef ARRAY_LEN
#define ARRAY_LEN(a) sizeof(a)/sizeof(a[0])
//sizeof(a[0])
#endif//ARRAY_LEN
#define PATH_LEN    1024
#define QUERY_LEN   2056
#define HEADERS_LEN 2056
#define BODY_LEN    4096
#define BUFFER_LEN  PATH_LEN+QUERY_LEN+HEADERS_LEN+BODY_LEN

#define SECRET_TOKEN "p&]]&'0Q9;IpM9[/-9==M&9&;]pQ'p/=&\\"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#ifdef __WIN32__
#   include <ws2tcpip.h>
#   include <winsock2.h>
#else
#   include <errno.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#endif
#include <unistd.h> // For close()

#define HTTP_STATUS_100 "Continue"
#define HTTP_STATUS_101 "Switching Protocols"
#define HTTP_STATUS_200 "OK"
#define HTTP_STATUS_201 "Created"
#define HTTP_STATUS_202 "Accepted"
#define HTTP_STATUS_203 "Non-Authoritative Information"
#define HTTP_STATUS_204 "No Content"
#define HTTP_STATUS_205 "Reset Content"
#define HTTP_STATUS_300 "Multiple Choices"
#define HTTP_STATUS_301 "Moved Permanently"
#define HTTP_STATUS_302 "Found"
#define HTTP_STATUS_303 "See Other"
#define HTTP_STATUS_305 "Use Proxy"
#define HTTP_STATUS_306 "(Unused)"
#define HTTP_STATUS_307 "Temporary Redirect"
#define HTTP_STATUS_400 "Bad Request"
#define HTTP_STATUS_402 "Payment Required"
#define HTTP_STATUS_403 "Forbidden"
#define HTTP_STATUS_404 "Not Found"
#define HTTP_STATUS_405 "Method Not Allowed"
#define HTTP_STATUS_406 "Not Acceptable"
#define HTTP_STATUS_408 "Request Timeout"
#define HTTP_STATUS_409 "Conflict"
#define HTTP_STATUS_410 "Gone"
#define HTTP_STATUS_411 "Length Required"
#define HTTP_STATUS_413 "Payload Too Large"
#define HTTP_STATUS_414 "URI Too Long"
#define HTTP_STATUS_415 "Unsupported Media Type"
#define HTTP_STATUS_417 "Expectation Failed"
#define HTTP_STATUS_426 "Upgrade Required"
#define HTTP_STATUS_500 "Internal Server Error"
#define HTTP_STATUS_501 "Not Implemented"
#define HTTP_STATUS_502 "Bad Gateway"
#define HTTP_STATUS_503 "Service Unavailable"
#define HTTP_STATUS_504 "Gateway Timeout"
#define HTTP_STATUS_505 "HTTP Version Not Supported"
#define UNKNOWN         "UNKNOWN"
#define HTTP_CONTENT_HTTP "Content-type:text/html;charset=UTF-8"
#define HTTP_CONTENT_TEXT "Content-type:text/plain;charset=UTF-8"

#ifndef socket_t
typedef intptr_t socket_t;
#endif//socket_t

#ifndef HTTP_SERVER_API
#define HTTP_SERVER_API
#endif//HTTP_SERVER_API

typedef struct {
    char *method;
    char *path;
    char *query;
    char *headers;
    char *body;
} request_t;

typedef struct {
    int   status;
    char *headers;
    char *body;
} response_t;

const response_t NOT_FOUND_RESPONSE = {404,HTTP_CONTENT_TEXT,HTTP_STATUS_404};
//const response_t ERROR_RESPONSE     = {500,HTTP_CONTENT_TEXT,HTTP_STATUS_500};

typedef response_t (*func) (request_t);

typedef struct {
    const char *route;
    func callback;
} route_t;

typedef struct {
    route_t *items;
    size_t count;
    size_t capacity;
} route_list_t;

HTTP_SERVER_API void print_error_and_exit();
HTTP_SERVER_API void *handle_client_connection(void* arg);
HTTP_SERVER_API void start_server(int port, route_list_t routes);
HTTP_SERVER_API void get_secure_token();

HTTP_SERVER_API char *prepare_response_string(response_t response) {
    char result[BUFFER_LEN];
    char *http_status;
    switch(response.status) {
    case 200: http_status = HTTP_STATUS_200; break;
    case 404: http_status = HTTP_STATUS_404; break;
    case 500: http_status = HTTP_STATUS_500; break;
    default:  http_status = UNKNOWN;         break;
    }
    sprintf(result, "HTTP/1.1 %d %s\r\n%s\r\n\r\n%s",
        response.status,
        http_status,
        response.headers,
        response.body   );
    return strdup(result);
}

#ifdef _SERVER_HTTP_IMPLEMENTATION

    static route_list_t global_routes = {0};

    HTTP_SERVER_API void get_secure_token() {
        
    }

    HTTP_SERVER_API void print_error_and_exit() {
        #ifdef __WIN32__

            DWORD error_code = WSAGetLastError(); // Get the error code
            char message_buffer[256];

            // Get the error message string
            FormatMessage(
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                error_code,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                message_buffer,
                sizeof(message_buffer),
                NULL
            );

            fprintf(stderr,"ERROR: Winsock error %ld: %s\n", error_code, message_buffer);
            exit(1);
        #else
            fprintf(stderr,"ERROR: %d: %s\n", errno, strerror(errno));        
        #endif
        
    }

    HTTP_SERVER_API void* handle_client_connection(void* arg) {
        socket_t client = (socket_t)arg;
        //printf("connected ... \n");
        char buf[BUFFER_LEN];
        int r = recv(client, buf, sizeof(buf),0);
        // int s=-1,e=0;
        
        char method[10]           = "";
        char path[PATH_LEN]       = "";
        char query[QUERY_LEN]     = "";
        char headers[HEADERS_LEN] = "";
        char body[BODY_LEN]       = "";

        int i=0, iota=0;
        while(i<r && iota<10 && buf[i]!=' ') method[iota++]=buf[i++];
        i++; iota=0;
        while(i<r && iota<PATH_LEN && buf[i]!=' ' && buf[i]!='?') path[iota++]=buf[i++];
        if (buf[i] == '?') {
            i++; iota=0;
            while(i<r && iota<QUERY_LEN && buf[i]!=' ') query[iota++]=buf[i++];
        }
        while (i<r && buf[i++]!='\n') {} // done first line
        iota=0;
        while (iota < HEADERS_LEN && i+4<r && !(buf[i+0]=='\r' && buf[i+1]=='\n' && buf[i+2]=='\r' && buf[i+3]=='\n')) {
            headers[iota++] = buf[i++];
        }
        iota=0;
        i+=4;
        while (i<r && iota<BODY_LEN) {
            body[iota++]=buf[i++];
        }

        printf("------------------------------------\n");
        printf("[method ]\n%s\n", method);
        printf("[path   ]\n%s\n", path);
        printf("[query  ]\n%s\n", query);
        printf("[headers]\n%s\n", headers);
        printf("[body   ]\n%s\n", body);
        printf("------------------------------------\n");

        struct sockaddr_in remote_addr;
        socklen_t addr_len = sizeof(remote_addr);
        getpeername(client, (struct sockaddr *)&remote_addr, &addr_len);
        // char time[20];
        // get_time(time);
        // printf("[%s] %s %s\n",time,inet_ntoa(remote_addr.sin_addr),path);

        response_t response = NOT_FOUND_RESPONSE;
        
        if (strcmp(headers, SECRET_TOKEN) == 0)
        {
            for(size_t i=0;i<global_routes.count;++i) {
                route_t r = global_routes.items[i];
                if(strcmp(r.route, path)==0) {
                    // printf("//bingo\n");
                    response = r.callback((request_t){method,path,query,headers,body});
                    break;
                }
            }
        }
        
        char *response_string = prepare_response_string(response);
        send(client, response_string, strlen(response_string), 0);
        shutdown(client,0x01);
        read(client, buf, 128);
        close(client);
        free(response_string);
        return NULL;
    }

    HTTP_SERVER_API void start_server(int port, route_list_t routes) {
        global_routes = routes;
        socket_t socketfd;
        
        struct sockaddr_in address;
        address.sin_family = AF_INET; //IPv4
        address.sin_port   = htons(port);
        address.sin_addr.s_addr = INADDR_ANY;
        // server.sin_zero   = 0;

        socklen_t addrlen = sizeof(address);

        #ifdef __WIN32__
            WORD versionWanted = MAKEWORD(2, 2);
            WSADATA wsaData;
            if(WSAStartup(versionWanted, &wsaData) != 0) {
                print_error_and_exit();
            }
        #endif
        
        if((socketfd = socket(AF_INET , SOCK_STREAM , 0 )) == 0) {
            print_error_and_exit();
        }

        if(bind(socketfd, (struct sockaddr*)&address, addrlen) != 0) {
            print_error_and_exit();
        }

        printf("server started on port %d !\n", port);
        
        if(listen(socketfd, 10) != 0) {
            print_error_and_exit();
        }
        
        printf("listening ... \n");
        socket_t client;
        while(1){  
            client = accept(socketfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            pthread_t thread;
            pthread_create(&thread,NULL,handle_client_connection,(void*)client);
        }
    }
#endif//_SERVER_HTTP_IMPLEMENTATION
#endif//_SERVER_H