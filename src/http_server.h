#pragma once
#ifndef _SERVER_H
#define _SERVER_H

#define PORT_HTTPS 1443
#define PORT_HTTP  8080 
#define DOMAIN_NAME "https://hos-hg.online:443/"


#ifndef SIGPIPE
#define	SIGPIPE	13	/* write on a pipe with no one to read it */
#endif//SIGPIPE

#ifndef SOL_TCP
#define SOL_TCP 6  /* socket options TCP level */
#endif//SOL_TCP

#ifndef TCP_USER_TIMEOUT
#define TCP_USER_TIMEOUT 18  /* how long for loss retry before timeout [ms] */
#endif//TCP_USER_TIMEOUT


#ifndef ARRAY_LEN
#define ARRAY_LEN(a) sizeof(a)/sizeof(a[0])
#endif//ARRAY_LEN

#define PATH_LEN    1024
#define QUERY_LEN   2056
#define HEADERS_LEN 2056
#define BODY_LEN    4096
#define BUFFER_LEN  10000

#define SECRET_TOKEN "p&]]&'0Q9;IpM9[/-9==M&9&;]pQ'p/=&\\"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#ifdef __WIN32__
#   define realpath(N,R) _fullpath((R),(N),PATH_MAX)
#   include <ws2tcpip.h>
#   include <winsock2.h>
#   include <windows.h>
#else
#   include <poll.h>
#   include <errno.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <netinet/tcp.h>
#endif
#include <signal.h>
#include <unistd.h> // For close()
#include "hash_map.h"
#define _UTILS_IMPLEMENTATION
#include "utils.h"
#include "tokenizer.h"

#include <openssl/ssl.h>
#include <openssl/types.h>
#include <openssl/err.h>
#include <fcntl.h>



#ifdef __WIN32__
#include <io.h>
#define F_OK 0
#define access _access
#endif
#include "pthread_pool.h"

#define HTTP_METHOD_GET    "GET"
#define HTTP_METHOD_POST   "POST"
#define HTTP_METHOD_PATCH  "PATCH"
#define HTTP_METHOD_PUT    "PUT"
#define HTTP_METHOD_DELETE "DELETE"
#define HTTP_METHOD_HEAD   "HEAD"

#define HTTP_STATUS_200 "OK"
#define HTTP_STATUS_403 "Forbidden"
#define HTTP_STATUS_404 "Not Found"
#define HTTP_STATUS_500 "Internal Server Error"

#define UNKNOWN         "UNKNOWN"
#define HTTP_CONTENT_HTTP "Content-type:text/html;charset=UTF-8"
#define HTTP_CONTENT_TEXT "Content-type:text/plain;charset=UTF-8"

#ifndef socket_t
typedef intptr_t socket_t;
#endif//socket_t

#ifndef HTTP_SRVR_API
#define HTTP_SRVR_API
#endif//HTTP_SRVR_API

#ifndef SHUT_RD
#define SHUT_RD 0
#endif//SHUT_RD

#ifndef SHUT_WR
#define SHUT_WR 1
#endif//SHUT_WR

#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif//SHUT_RDWR


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

typedef struct {
    socket_t  sockfd;
    int       port;
    SSL      *ssl;
    SSL_CTX  *ctx;
    char     *ip;
    // char     *username;
    // uchar_t  *authorizations;
    int retry;
} client_t;

const response_t NOT_FOUND_RESPONSE = {404,HTTP_CONTENT_TEXT,HTTP_STATUS_404};
//const response_t ERROR_RESPONSE     = {500,HTTP_CONTENT_TEXT,HTTP_STATUS_500};

typedef void (*rqFn) (const request_t,client_t client);

typedef struct {
    char *route;
    rqFn  handler;
} route_t;

typedef struct {
    route_t *items;
    size_t   count;
    size_t   capacity;
} route_list_t;

// -- INTERFACE

HTTP_SRVR_API void  HTTP_SRVR_print_net_error(int _ref);
HTTP_SRVR_API void *HTTP_SRVR_handle_client(void* arg);
HTTP_SRVR_API void  HTTP_SRVR_start(int port, route_list_t *routes);
HTTP_SRVR_API void  HTTP_SRVR_get_secure_token();
HTTP_SRVR_API bool  HTTP_SRVR_file_exists(const char *file_path);
HTTP_SRVR_API void  HTTP_SRVR_send_file(char *req_path, client_t client);
HTTP_SRVR_API void  HTTP_SRVR_get_content_type(char *content_type, const char *file_path);
HTTP_SRVR_API void  HTTP_SRVR_get_socket_ip(char *dest, int socket_fd);
HTTP_SRVR_API char *HTTP_SRVR_get_ssl_err_name(int err) ;

// to make transition between http/https seamless
HTTP_SRVR_API int HTTP_SRVR_send(client_t s, const char *buf,int len,int flags);
HTTP_SRVR_API int HTTP_SRVR_recv(client_t s, char *buf,int len,int flags);

// --- IMPLEMENTATION

// #ifndef MSG_NOSIGNAL
// #define MSG_NOSIGNAL 0x4000 /* Do not generate SIGPIPE.  */
// #endif//MSG_NOSIGNAL


#ifdef _HTTP_SRVR_IMPLEMENTATION

#define ASSETS_PATH "/home/fms/fms/assets"
#define ASSETS_PATH_LENGTH strlen(ASSETS_PATH)

    static volatile bool keep_running = true;

    static route_list_t global_routes = {0};
    static hash_map_t routes_map = {0};

    /** Returns true on success, or false if there was an error */
    bool set_socket_blocking_enabled(int fd, bool blocking) {
        if (fd < 0) return false;

        #ifdef _WIN32
            unsigned long mode = blocking ? 0 : 1;
            return (ioctlsocket(fd, FIONBIO, &mode) == 0);
        #else
            int flags = fcntl(fd, F_GETFL, 0);
            if (flags == -1) return false;
            flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
            return (fcntl(fd, F_SETFL, flags) == 0);
        #endif
    }

    HTTP_SRVR_API void  HTTP_SRVR_get_socket_ip(char *ip, int socket_fd) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        getpeername(socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        inet_ntop(AF_INET, &(client_addr.sin_addr), ip, INET_ADDRSTRLEN);
    }

    
    HTTP_SRVR_API char *HTTP_SRVR_get_ssl_err_name(int err) {
        switch(err){
        case SSL_ERROR_NONE                 : return "SSL_ERROR_NONE"; 
        case SSL_ERROR_SSL                  : return "SSL_ERROR_SSL"; 
        case SSL_ERROR_WANT_READ            : return "SSL_ERROR_WANT_READ"; 
        case SSL_ERROR_WANT_WRITE           : return "SSL_ERROR_WANT_WRITE"; 
        case SSL_ERROR_WANT_X509_LOOKUP     : return "SSL_ERROR_WANT_X509_LOOKUP"; 
        case SSL_ERROR_SYSCALL              : return "SSL_ERROR_SYSCALL";                                         
        case SSL_ERROR_ZERO_RETURN          : return "SSL_ERROR_ZERO_RETURN"; 
        case SSL_ERROR_WANT_CONNECT         : return "SSL_ERROR_WANT_CONNECT"; 
        case SSL_ERROR_WANT_ACCEPT          : return "SSL_ERROR_WANT_ACCEPT"; 
        case SSL_ERROR_WANT_ASYNC           : return "SSL_ERROR_WANT_ASYNC"; 
        case SSL_ERROR_WANT_ASYNC_JOB       : return "SSL_ERROR_WANT_ASYNC_JOB"; 
        case SSL_ERROR_WANT_CLIENT_HELLO_CB : return "SSL_ERROR_WANT_CLIENT_HELLO_CB"; 
        case SSL_ERROR_WANT_RETRY_VERIFY    : return "SSL_ERROR_WANT_RETRY_VERIFY"; 
        };
        return "UNKNOWN";
    }

    int get_socket_readiness(client_t s) {
        int ret = 0;
        errno = 0;
        #ifdef __WIN32__
            WSAPOLLFD fds[1];
            fds[0].fd = s.sockfd;
            fds[0].events = POLLIN | POLLOUT; // Check for normal data to read or error conditions
            ret = WSAPoll(fds, 1, -1);
        #else
            struct pollfd pfd;
            pfd.fd = s.sockfd; // The socket descriptor to monitor
            pfd.events = POLLIN | POLLOUT;    // Check for read and write readiness
            ret = poll(&pfd, 1, -1); // -1: infinit amount of time, 0: immediate, positive: specified time in ms
        #endif
        if (ret == -1 || errno != 0) {
            perror("poll error");
            HTTP_SRVR_print_net_error(99);
            return -1;
        } else if (ret == 0) {
            wrn("Socket timed out to client [%s], no events.\n", s.ip);
            return 0;
        }
        return ret;
    }
        
    HTTP_SRVR_API int HTTP_SRVR_send(client_t s,const char *buf,int len,int flags) {
        if (get_socket_readiness(s) < 1) return -1;
        if (s.ssl !=NULL && s.port == PORT_HTTPS) {
            return SSL_write(s.ssl,buf,len);
        }
        return send(s.sockfd,buf,len,flags);
    }
    
    // WORKING // HTTP_SRVR_API int HTTP_SRVR_recv(client_t s, char *buf,int len,int flags) {
    // WORKING //     if (get_socket_readiness(s) < 1) return -1;
    // WORKING //     if (s.ssl != NULL && s.port == PORT_HTTPS)
    // WORKING //         return SSL_read(s.ssl,buf,len);
    // WORKING //     return recv(s.sockfd,buf,len,flags); //flags);        
    // WORKING // }

    HTTP_SRVR_API int HTTP_SRVR_recv(client_t s, char *buf,int len,int flags) {
        (void)flags;
        if (get_socket_readiness(s) < 1) return -1;
        if (s.ssl != NULL && s.port == PORT_HTTPS) {
            return SSL_read(s.ssl,buf,len);
        }
        return recv(s.sockfd,buf,len,flags);
        // set_socket_blocking_enabled(s.sockfd,true);
        // int n = 0;
        // int r = 0;
        // int end[4] = {0,0,0,0};
        // for(;;) {
        //     char tmp[1] = {0};
        //     n = recv(s.sockfd,tmp,1,0);
        //     if (n < 0 && errno>0) {
        //         perror("ERROR reading from socket");
        //         break;
        //     }
        //     // if (tmp[0] == '\x3') break;
        //     // printf("%d|(%d)",(int)tmp[0],n);
        //     buf[r++] = tmp[0];
        //     if (r >= len) {
        //         //err("buffer length %d is shorter than expected %d", len, sizeof(buf));
        //         break;
        //     }
        //     end[0] = end[1];
        //     end[1] = end[2];
        //     end[2] = end[3];
        //     end[3] = tmp[0];
        //     // printf("\n'%d,%d,%d,%d'\n",end[0],end[1],end[2],end[3]);
        //     if(end[0]==13 && end[1]==10 && end[2]==13 && end[3]==10) {
        //         printf("got terminate signal \n");
        //         break;
        //     }
        //     printf("reading...\n");
        // } 
        // shutdown(s.sockfd,SHUT_RD);
        // printf("\n---------------------------------------------\n");
        // printf("'%s'\n",buf);
        // printf("---------------------------------------------\n");
        // return r;
    }


    HTTP_SRVR_API void HTTP_SRVR_get_content_type(char *content_type, const char *file_path) {
        size_t l;
        char *ext;
        
        if (file_path == NULL || (l=strlen(file_path)) == 0) return;

        for(int i=l-1;i>=0;--i) {
            if (file_path[i]=='.') {
                ext = strdup(file_path)+i+1;
                break;
            }
        }

        if      (strcasecmp(ext,"html" ) == 0 
             ||  strcasecmp(ext,"htm"  ) == 0) sprintf(content_type, "Content-Type: text/html");
        else if (strcasecmp(ext,"js"   ) == 0   
             ||  strcasecmp(ext,"mjs"  ) == 0) sprintf(content_type, "Content-Type: text/javascript");
        else if (strcasecmp(ext,"css"  ) == 0) sprintf(content_type, "Content-Type: text/css");
        else if (strcasecmp(ext,"txt"  ) == 0) sprintf(content_type, "Content-Type: text/plain");
        else if (strcasecmp(ext,"ttf"  ) == 0) sprintf(content_type, "Content-Type: font/ttf");
        else if (strcasecmp(ext,"zip"  ) == 0
             ||  strcasecmp(ext,"rtf"  ) == 0
             ||  strcasecmp(ext,"pdf"  ) == 0
             ||  strcasecmp(ext,"xml"  ) == 0
             ||  strcasecmp(ext,"wasm" ) == 0
             ||  strcasecmp(ext,"json" ) == 0) sprintf(content_type, "Content-Type: application/%s", ext);
        else if (strcasecmp(ext,"png"  ) == 0
             ||  strcasecmp(ext,"webp" ) == 0
             ||  strcasecmp(ext,"ico"  ) == 0
             ||  strcasecmp(ext,"jpg"  ) == 0
             ||  strcasecmp(ext,"jpeg" ) == 0
             ||  strcasecmp(ext,"gif"  ) == 0
             ||  strcasecmp(ext,"bmp"  ) == 0
             ||  strcasecmp(ext,"tif"  ) == 0
             ||  strcasecmp(ext,"tiff" ) == 0) sprintf(content_type, "Content-Type: image/%s", ext); 
        else if (strcasecmp(ext,"svg"  ) == 0) sprintf(content_type, "Content-Type: image/svg+xml");
        else if (strcasecmp(ext,"otf"  ) == 0 
             ||  strcasecmp(ext,"ttf"  ) == 0 
             ||  strcasecmp(ext,"eot"  ) == 0 
             ||  strcasecmp(ext,"woff" ) == 0 
             ||  strcasecmp(ext,"woff2") == 0) sprintf(content_type, "Content-Type: font/%s", ext); 

        //return strdup(content_type);
    }

    // HTTP_SRVR_API void get_static_file_response(response_t *response, const char *file_path) {
    //     if (file_path == NULL || strlen(file_path) == 0 || !HTTP_SRVR_file_exists(file_path)) {
    //         response->body="incorrect request";
    //         response->status=500;
    //         response->headers="Status: 500";
    //         return;
    //     }
    //     char os_path[1024] = ".";
    //     strcat(os_path, file_path);
    //     if (!HTTP_SRVR_file_exists(os_path)) {
    //         response->body="incorrect request [file not found]";
    //         response->status=500;
    //         response->headers="Status: 500";
    //         return;
    //     }
    //     inf("getting static file contents: '%s'", os_path);

    //     //char buffer[553768];
    //     FILE *fptr = fopen(os_path, "rb");
    //     if (fptr != NULL) {
    //         fseek(fptr, 0, SEEK_END);
    //         long size = ftell(fptr);
    //         fseek(fptr, 0, SEEK_SET);
    //         // printf("size=%lu\n", size);

    //         char *buffer = (char*)malloc(size+1);
    //         int r = fread(buffer,size,1,fptr);

    //         printf("r=%d\n", r);
    //         if(r>0) {
    //             buffer[size-1]='\0';
    //             //printf("strlen(buffer)=%zu\n", strlen(buffer));
    //             char content_type[100] = {0};
    //             HTTP_SRVR_get_content_type(content_type, file_path);
    //             response->headers = strdup(content_type);
    //             response->body = strdup(buffer);
    //             response->status = 200;
    //             // free(buffer);
    //             return;
    //         }
    //     }
    //     fclose(fptr);
    //     wrn("404 unable to get static file '%s'", file_path);
    //     response->body = "RESOURCE WAS NOT FOUND";
    //     response->status = 404;
    // }

    HTTP_SRVR_API bool HTTP_SRVR_file_exists(const char *file_path) {
        if (file_path == NULL || strlen(file_path) == 0) return false;
        if (file_path[0] == '.') return access(file_path, F_OK) == 0;
        char os_path[1024] = {0};
        sprintf(os_path,".%s", file_path);
        // printf("os_path: '%s'", os_path);
        bool status = access(os_path, F_OK) == 0;
        return status;
    }

    void get_status_message(char *http_status,int status) {
        switch(status) {
        case 200: sprintf(http_status,"%s", HTTP_STATUS_200); break;
        case 403: sprintf(http_status,"%s", HTTP_STATUS_403); break;
        case 404: sprintf(http_status,"%s", HTTP_STATUS_404); break;
        case 500: sprintf(http_status,"%s", HTTP_STATUS_500); break;
        default:  sprintf(http_status,"%s", UNKNOWN);         break;
        }
    }

    HTTP_SRVR_API void prepare_response_string(char *response_string,response_t response) {
        char http_status[100];
        get_status_message(http_status,response.status);        
        sprintf(response_string, "HTTP/1.1 %d %s\r\n%s\r\n\r\n%s",
            response.status,
            http_status,
            response.headers,
            response.body   );
    }


    HTTP_SRVR_API void HTTP_SRVR_get_secure_token() {
        
    }

    HTTP_SRVR_API void HTTP_SRVR_print_net_error(int _ref) {
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

            fprintf(stderr,"ERROR[%d]: Winsock error %ld: %s\n",_ref, error_code, message_buffer);
            //exit(1);
        #else
            if(errno != 0) err("[NET/0/0x%03x] ERROR: %d: %s\n",_ref, errno, strerror(errno));        
        #endif
        
    }

    // static const char *path_end[] = {" ","?","\n"};
    // static const char *qury_end[] = {" ","\n"};
    // static const char *hedr_end[] = {"\r\n\r\n"};

    // void parse_http_request(request_t *request,int r, char *buf) {
    //     assertf (request != NULL, "request cannot be null !");
    //     assertf (r >= 0 && r <= strlen(buf), "the length of the buffer (%d / %d) is unknown !",r, strlen(buf));
    //     assertf (buf != NULL || strlen(buf) == 0, "buffer cannot be empty or null !");
    //     tokenizer_t tok = {buf,0};

    //     request->method  = tok_next(&tok); tok_chop_spaces(&tok);
    //     request->path    = tok_take_until(&tok,ARRAY_LEN(path_end),path_end);
    //     if(tok.string[tok.pos]=='?') {
    //         tok.pos++;
    //         request->query   = tok_take_until(&tok,ARRAY_LEN(qury_end),qury_end); tok_drop_until(&tok,"\n");
    //     }
    //     request->headers = tok_take_until(&tok,ARRAY_LEN(hedr_end),hedr_end); tok_chop_spaces(&tok);
    //     request->body    = tok_remaining(&tok);

    //     if(stricmp(request->method, HTTP_METHOD_GET )==0) request->method = HTTP_METHOD_GET;
    //     if(stricmp(request->method, HTTP_METHOD_POST)==0) request->method = HTTP_METHOD_POST;

    //     inf("------------------------------------");
    //     inf("[method ]: '%s'", request->method);
    //     inf("[path   ]: '%s'", request->path);
    //     inf("[query  ]: '%s'", request->query);
    //     // inf("[headers]: '%s'", request->headers);
    //     inf("[body   ]: '%s'", request->body);
    //     inf("------------------------------------");
    // }

    void parse_http_request(request_t *request,int r, char *buf) {
        assertf (request != NULL, "request cannot be null !");
        //assertf (r >= 0 && r <= strlen(buf), "the length of the buffer (%d / %d) is unknown !",r, (int)strlen(buf));
        assertf (buf != NULL || strlen(buf) == 0, "buffer cannot be empty or null !");

        // printf("------------------------------------------------------------\n");
        // printf("%s\n",buf);
        // printf("------------------------------------------------------------\n");
        char method[15]           = "";
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

             if(stricmp(method, HTTP_METHOD_GET )==0) sprintf(method,"%s", HTTP_METHOD_GET);
        else if(stricmp(method, HTTP_METHOD_POST)==0) sprintf(method,"%s", HTTP_METHOD_POST);
        else sprintf(method,"UNSUPPORTED");

        
        request->method  = strdup(method);
        request->path    = strdup(path);
        request->query   = strdup(query);
        request->headers = strdup(headers);
        request->body    = strdup(body);        
    }

    void send_headers(int status, const char *content_type, const char *other_headers, client_t client) {
        HTTP_SRVR_send(client,"HTTP/1.1",8,0);
        char status_message[100];
        sprintf(status_message," %d ",status);
        HTTP_SRVR_send(client,status_message,strlen(status_message),0);
        get_status_message(status_message,status);
        HTTP_SRVR_send(client,status_message,strlen(status_message),0);
        HTTP_SRVR_send(client,"\r\n",2,0);
        if (content_type != NULL) {
            HTTP_SRVR_send(client,content_type,strlen(content_type),0);
            HTTP_SRVR_send(client,"\r\n",2,0);
        }
        if (other_headers != NULL) {
            HTTP_SRVR_send(client,other_headers,strlen(other_headers),0);
        }
        HTTP_SRVR_send(client,"Connection: close\r\n\r\n",21,0);
    }

    #define BLEN 1024
    HTTP_SRVR_API void HTTP_SRVR_send_file(char *req_path, client_t client) {
        //inf("here: %s [%s]",__func__,req_path);
        char content_type[50];
        HTTP_SRVR_get_content_type(content_type, req_path);
        char header[200] = {0};
        
        int r=0;
        char buffer[BLEN];
        FILE *fp = fopen(req_path,"rb");
        size_t size = 0;
        if (fseek(fp, 0, SEEK_END) == 0) {
            size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
        }
        if (strncmp("font", content_type, 4) != 0 || strncmp("image", content_type, 5) != 0) {
            char *expire = DATE_H_date_format(DATE_H_add_to_date(DATE_H_now(),DATE_H_DAY,1),"ddd, dd MMM yyyy HH:mm:ss");
            sprintf(header,"Expires: %s GMT\r\nCache-Control: public, max-age=31536001, immutable\r\n",expire);
        }
        if (size > 0) {
            char tmp[strlen(header)+50];
            sprintf(tmp, "%sContent-Length: %ld\r\n", header, size);
            sprintf(header, "%s", tmp);
        }
        send_headers(200,content_type,header,client);

        while (fp != NULL && (r=fread(buffer,1,BLEN,fp))>0) {
            HTTP_SRVR_send(client,buffer,r,0);
        }

        fclose(fp);
        
        if (client.port == PORT_HTTPS && client.ssl != NULL) {
            SSL_shutdown(client.ssl);
            SSL_read(client.ssl, buffer, 128);
        }

        shutdown(client.sockfd,SHUT_WR);
        read(client.sockfd, buffer, 128);
        close(client.sockfd);
    }


    void* hsts_handler(void *arg) {
        client_t client = *(client_t*)arg;
        free(arg);
        socket_t client_sockfd = client.sockfd;
        if (get_socket_readiness(client) < 0) return (void*)-1;
        char b[516] =
            "HTTP/1.1 302 Found\r\n"
            "Content-Type: text/plain; charset=utf-8\r\n"
            "Strict-Transport-Security: max-age=31536000; includeSubDomains; preload\r\n"
            "Location: "DOMAIN_NAME"\r\n"
            "Cache-Control: no-cache\r\n"
            "\r\n"
            "Moved to "DOMAIN_NAME
            ;
        HTTP_SRVR_send(client,b,strlen(b),0);
        shutdown(client_sockfd,SHUT_WR);
        read(client_sockfd, b, 128);
        close(client_sockfd);        
        return NULL;

    }

    void configure_socket(int sockfd) {
        const char on = 1;
        // const char off = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR , &on, sizeof(on));
        #ifdef SO_REUSEPORT
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT , &on, sizeof(on));
        #endif
        setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
        setsockopt(sockfd, SOL_SOCKET, SO_LINGER   , &on, sizeof(on));
        struct timeval timeout;      
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof timeout) < 0)
            err("[%d] setsockopt failed",1);

        if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof timeout) < 0)
            err("[%d] setsockopt failed",2); 


        //enabling keep-alive options
        int yes = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&yes, sizeof(int));

        int idle = 1;
        setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, (const char*)&idle, sizeof(int));

        int interval = 1;
        setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL, (const char*)&interval, sizeof(int));

        int maxpkt = 100;
        setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPCNT, (const char*)&maxpkt, sizeof(int));
    }


    HTTP_SRVR_API void *HTTP_SRVR_handle_client(void *arg) {

        client_t client = *(client_t*)arg;
        free(arg);

        if (get_socket_readiness(client) < 1) return NULL; // non-zero error code
        
        if (client.port == PORT_HTTPS) {
            // Create SSL object for client
            SSL *ssl = SSL_new(client.ctx);
            SSL_set_verify(ssl, SSL_VERIFY_NONE, NULL);
        	SSL_set_verify_depth(ssl, 4);
            SSL_set_fd(ssl, client.sockfd);
            client.ssl = ssl;
            int ssl_err = SSL_accept(ssl);
            if(ssl_err < 0) {
                SSL_shutdown(ssl);
                SSL_free(ssl);
                char buf[128];
                shutdown(client.sockfd, SHUT_WR);
                read(client.sockfd,buf,128);
                close(client.sockfd);
                return NULL; // non-zero error code
            } 
        }

        dbg("connected ... ");
        char buf[BUFFER_LEN] = {0};
        int bytes = HTTP_SRVR_recv(client, buf, sizeof(buf),0);

        
        request_t req = {0};
        parse_http_request(&req,bytes,buf);

                    // struct sockaddr_in remote_addr;
                    // socklen_t addr_len = sizeof(remote_addr);
                    // getpeername(client.sockfd, (struct sockaddr *)&remote_addr, &addr_len);

        response_t response = NOT_FOUND_RESPONSE;
        
        if (req.path != NULL && strlen(req.path)>0) {
            // printf("\n\n------------------\nexists [%s]: %d\n----------------\n",req.path, HTTP_SRVR_file_exists(req.path));
            char route_path[1024] = {0};
            sprintf(route_path,"%s:%s",req.method,req.path);
            inf("client [%s] requested : %s", client.ip, route_path);
            int index = hash_map_get(&routes_map, route_path);
            route_t route = global_routes.items[index];
            // printf("\n\n------------------\nexists [%s]: %d\n----------------\n",req.path, HTTP_SRVR_file_exists(req.path));
            
            dbg(" router [%s]", route.route);
            
            if(strcasecmp(route.route, route_path)==0) {
                //if(strcmp(route.route,"GET:/") != 0) 
                //    send_headers(200,"Content-Type: text/html",NULL,client);                
                route.handler(req, client);
                
                if (client.port == PORT_HTTPS && client.ssl != NULL) {
                    SSL_shutdown(client.ssl);
                    char buffer[128];
                    SSL_read(client.ssl, buffer, 128);
                }
                shutdown(client.sockfd,SHUT_WR);
                read(client.sockfd, buf, 128);
                close(client.sockfd);
                return NULL;  //non-zero error code
            }
            else if ( HTTP_SRVR_file_exists(req.path)) {
                char path[PATH_MAX] = ".";
                strcat(path,req.path);
                char *resolved_path = malloc(PATH_MAX);
                realpath(path, resolved_path);
                sprintf(path,"%s",resolved_path);
                free(resolved_path);
                if(strncmp(path,ASSETS_PATH,ASSETS_PATH_LENGTH) != 0) {
                    response.status = 403;
                    response.body = "NOT AUTHORIZED";   
                } else {
                    HTTP_SRVR_send_file(path, client);
                    return NULL;
                }
            }
        }


        // if (strcmp(headers, SECRET_TOKEN) == 0)
        // {
        // }

        printf("------------------------------------------------------------\n");
        printf("NOT FOUND : '%s'\n", req.path);
        printf("------------------------------------------------------------\n");
        
        char response_string[2048]; 
        prepare_response_string(response_string,response);
        HTTP_SRVR_send(client, response_string, strlen(response_string), 0);
        
        if (client.port == PORT_HTTPS && client.ssl != NULL) {
            SSL_shutdown(client.ssl);
            char buffer[128];
            SSL_read(client.ssl, buffer, 128);
        }
        
        shutdown(client.sockfd, SHUT_WR);
        read(client.sockfd, buf, 128);
        close(client.sockfd);
        return NULL;
    }

    
    // void intHandler(int _) {
    //     keep_running = false;
    // }

    typedef void* func_t (void*);

    typedef struct {
        func_t *func;
        void *client;
    } threadpool_item;

    

    


    HTTP_SRVR_API void HTTP_SRVR_start(int port, route_list_t *routes) {
        signal(SIGPIPE, SIG_IGN);
        bool is_default_handler = (routes != NULL);
        if (is_default_handler) {
            global_routes = *routes;
            hash_map_init(&routes_map);

            for(size_t i=0;i<global_routes.count;i++){
                inf(" -- [adding route] -- '%s'", global_routes.items[i].route);
                hash_map_add(&routes_map,global_routes.items[i].route,i);
            }
        }

        socket_t server_sockfd;
        
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
                HTTP_SRVR_print_net_error(0);
            }
        #endif
        
        if((server_sockfd = socket(AF_INET , SOCK_STREAM , 0 )) == 0) {
            HTTP_SRVR_print_net_error(1);
        }

        configure_socket(server_sockfd);

        if(bind(server_sockfd, (struct sockaddr*)&address, addrlen) != 0) {
            HTTP_SRVR_print_net_error(2);
        }

        inf("server started on port [%d] !\n", port);
        
        if(listen(server_sockfd, 200) != 0) {
            HTTP_SRVR_print_net_error(3);
        }

        dbg("listening ...");
        socket_t client_sockfd;

        SSL_CTX *ctx = {0};

        if (port==PORT_HTTPS) {
            inf("Initialize OpenSSL %p",NULL);
            // Initialize OpenSSL
            SSL_library_init();
            SSL_load_error_strings();
            OpenSSL_add_all_algorithms();

            ctx = SSL_CTX_new(TLS_server_method());
            SSL_CTX_set_options(ctx, SSL_OP_ALL);
            // Load certificate and private key into ctx
            if (!ctx) {
                err("SSL ERR: %s","SSL/0");

                ERR_print_errors_fp(stderr);
                // Handle error
            }
            if (SSL_CTX_use_certificate_file(ctx, "./ssl/cert.pem", SSL_FILETYPE_PEM) <= 0) {
                    err("SSL ERR: %s","SSL/1");

                ERR_print_errors_fp(stderr);
                // Handle error
            }
            if (SSL_CTX_use_PrivateKey_file(ctx, "./ssl/key.pem", SSL_FILETYPE_PEM) <= 0) {
                    err("SSL ERR: %s","SSL/2");

                ERR_print_errors_fp(stderr);
                // Handle error
            }
            if (!SSL_CTX_check_private_key(ctx)) {
                err("SSL ERR: %s : %s","SSL/3","Private key does not match the certificate");
                // Handle error
            }

            unsigned char cache_id[] = "my_server_app";
            SSL_CTX_set_session_id_context(ctx, (void *)cache_id, sizeof(cache_id));
            SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_SERVER);
            SSL_CTX_sess_set_cache_size(ctx, 1024); // Cache up to 1024 sessions
            SSL_CTX_set_timeout(ctx, 3600); // Sessions expire after 1 hour
        }

        while(keep_running) {       
            client_sockfd = accept(server_sockfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            if (client_sockfd < 1) {
                HTTP_SRVR_print_net_error(9);
                printf("\n~~~~~~~\ndenied\n~~~~~~~\n");
                shutdown(client_sockfd,SHUT_RDWR);
                close(client_sockfd);
                continue;
            }

            configure_socket(client_sockfd);


            //printf("[connect...]\n");
            
            char ip[INET_ADDRSTRLEN];
            HTTP_SRVR_get_socket_ip(ip,client_sockfd);
            client_t *clientArg = malloc(sizeof(client_t));
            //{client_sockfd,port,NULL,ctx,ip,0}
            clientArg->ctx=ctx;
            clientArg->sockfd=client_sockfd;
            clientArg->port=port;
            clientArg->ip=strdup(ip);
            clientArg->retry=0;
            pthread_t thread;
            if (is_default_handler){
                pthread_create(&thread,NULL,HTTP_SRVR_handle_client,(void*)clientArg);
            } else {
                pthread_create(&thread,NULL,hsts_handler,(void*)clientArg);
            }
        }

        shutdown(server_sockfd, SHUT_RDWR);
        close(server_sockfd);
    }
#endif//_HTTP_SRVR_IMPLEMENTATION
#endif//_SERVER_H