#pragma once
#ifndef _SERVER_H
#define _SERVER_H


#ifndef ARRAY_LEN
#define ARRAY_LEN(a) sizeof(a)/sizeof(a[0])
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
#include <signal.h>
#include <unistd.h> // For close()
#include "hash_map.h"
#define _UTILS_IMPLEMENTATION
#include "utils.h"
#include "tokenizer.h"

#include <openssl/ssl.h>
#include <openssl/types.h>
#include <openssl/err.h>


#ifdef __WIN32__
#include <io.h>
#define F_OK 0
#define access _access
#endif


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

typedef struct {
    socket_t sockfd;
    int port;
    SSL *ssl;
    SSL_CTX *ctx;
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

HTTP_SERVER_API void print_net_error(int _ref);
HTTP_SERVER_API void *handle_client_connection(void* arg);
HTTP_SERVER_API void start_server(int port, route_list_t *routes);
HTTP_SERVER_API void get_secure_token();
HTTP_SERVER_API bool file_exists(const char *file_path);
HTTP_SERVER_API void get_static_file_response(response_t *response, const char *file_path);
HTTP_SERVER_API void get_content_type(char *content_type, const char *file_path);

char *get_ssl_err_name(int err) ;

// to make transition between http/https seamless
HTTP_SERVER_API int net_send(client_t s, const char *buf,int len,int flags);
HTTP_SERVER_API int net_recv(client_t s, char *buf,int len,int flags);

// --- IMPLEMENTATION

// #ifndef MSG_NOSIGNAL
// #define MSG_NOSIGNAL 0x4000 /* Do not generate SIGPIPE.  */
// #endif//MSG_NOSIGNAL


#ifdef _SERVER_HTTP_IMPLEMENTATION

    static route_list_t global_routes = {0};
    static hash_map_t routes_map = {0};

    
    char *get_ssl_err_name(int err) {
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


    HTTP_SERVER_API int net_send(client_t s,const char *buf,int len,int flags) {
        if (s.ssl !=NULL && s.port == 443)
            return SSL_write(s.ssl,buf,len);
        return send(s.sockfd,buf,len,flags);
    }
    
    HTTP_SERVER_API int net_recv(client_t s, char *buf,int len,int flags) {
        if (s.ssl != NULL && s.port == 443)
            return SSL_read(s.ssl,buf,len-1);
        return recv(s.sockfd,buf,len-1,flags);        
    }

    HTTP_SERVER_API void get_content_type(char *content_type, const char *file_path) {
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

    HTTP_SERVER_API void get_static_file_response(response_t *response, const char *file_path) {
        if (file_path == NULL || strlen(file_path) == 0 || !file_exists(file_path)) {
            response->body="incorrect request";
            response->status=500;
            response->headers="Status: 500";
            return;
        }
        char os_path[1024] = ".";
        strcat(os_path, file_path);
        if (!file_exists(os_path)) {
            response->body="incorrect request [file not found]";
            response->status=500;
            response->headers="Status: 500";
            return;
        }
        inf("getting static file contents: '%s'", os_path);

        //char buffer[553768];
        FILE *fptr = fopen(os_path, "rb");
        if (fptr != NULL) {
            fseek(fptr, 0, SEEK_END);
            long size = ftell(fptr);
            fseek(fptr, 0, SEEK_SET);
            // printf("size=%lu\n", size);

            char *buffer = (char*)malloc(size+1);
            int r = fread(buffer,size,1,fptr);

            printf("r=%d\n", r);
            if(r>0) {
                buffer[size-1]='\0';
                //printf("strlen(buffer)=%zu\n", strlen(buffer));
                char content_type[100] = {0};
                get_content_type(content_type, file_path);
                response->headers = strdup(content_type);
                response->body = strdup(buffer);
                response->status = 200;
                // free(buffer);
                return;
            }
        }
        fclose(fptr);
        wrn("404 unable to get static file '%s'", file_path);
        response->body = "RESOURCE WAS NOT FOUND";
        response->status = 404;
    }

    HTTP_SERVER_API bool file_exists(const char *file_path) {
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

    HTTP_SERVER_API void prepare_response_string(char *response_string,response_t response) {
        char http_status[100];
        get_status_message(http_status,response.status);        
        sprintf(response_string, "HTTP/1.1 %d %s\r\n%s\r\n\r\n%s",
            response.status,
            http_status,
            response.headers,
            response.body   );
    }


    HTTP_SERVER_API void get_secure_token() {
        
    }

    HTTP_SERVER_API void print_net_error(int _ref) {
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
            err("[NET/0/0x%03x] ERROR: %d: %s\n",_ref, errno, strerror(errno));        
        #endif
        
    }

    void parse_http_request(request_t *request,int r, char *buf) {
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

        // printf("------------------------------------\n");
        // printf("[method ]: '%s'\n", method);
        // printf("[path   ]: '%s'\n", path);
        // printf("[query  ]: '%s'\n", query);
        // printf("[headers]: '%s'\n", headers);
        // printf("[body   ]: '%s'\n", body);
        // printf("------------------------------------\n");

        
        request->method  = strdup(method);
        request->path    = strdup(path);
        request->query   = strdup(query);
        request->headers = strdup(headers);
        request->body    = strdup(body);        
    }

    void send_headers(int status, const char *content_type, const char *other_headers, client_t client) {
        net_send(client,"HTTP/1.1",8,0);
        char status_message[100];
        sprintf(status_message," %d ",status);
        net_send(client,status_message,strlen(status_message),0);
        get_status_message(status_message,status);
        net_send(client,status_message,strlen(status_message),0);
        net_send(client,"\r\n",2,0);
        if (content_type != NULL) {
            net_send(client,content_type,strlen(content_type),0);
            net_send(client,"\r\n",2,0);
        }
        if (other_headers != NULL) {
            net_send(client,other_headers,strlen(other_headers),0);
        }
        net_send(client,"Expires:-1\r\nAge:0\r\nConnection: close\r\n\r\n",40,0);
    }

    void serve_static_file(char *req_path, client_t client) {
        //inf("here: %s [%s]",__func__,req_path);
        char content_type[50];
        get_content_type(content_type, req_path);
        send_headers(200,content_type,NULL,client);

        int r=0;
        char buffer[256];
        FILE *fp = fopen(req_path,"rb");
        if (fp != NULL)
        {
            while ((r=fread(buffer,1,256,fp))>0) {
                //inf("sending buffer len = %d", r);
                net_send(client,buffer,r,0);
            }
        }
        fclose(fp);
        
        if (client.port == 443 && client.ssl != NULL) {
            SSL_shutdown(client.ssl);
            SSL_read(client.ssl, buffer, 128);
            //SSL_free(client.ssl);            
        }

        shutdown(client.sockfd,0x01);
        read(client.sockfd, buffer, 128);
        close(client.sockfd);
    }

    void *hsts_handler(void *arg) {
        socket_t client_sockfd = (socket_t)arg;
        char b[516] =
            "HTTP/1.1 302 Found\r\n"
            "Content-Type: text/plain; charset=utf-8\r\n"
            "Strict-Transport-Security: max-age=31536000; includeSubDomains; preload\r\n"
            "Location: https://your-web-server-domain-name-here.com/\r\n"
            "Cache-Control: no-cache\r\n"
            "\r\n"
            "Moved Permanently. Redirecting to https://your-web-server-domain-name-here/"
            ;
        send(client_sockfd,b,strlen(b),0);
        shutdown(client_sockfd,0x01);
        read(client_sockfd, b, 128);
        close(client_sockfd);
        
        return NULL;

    }

    HTTP_SERVER_API void* handle_client_connection(void *arg) {

        client_t *clientArg = (client_t*)arg;
        client_t client = {
            .port = clientArg->port,
            .sockfd = clientArg->sockfd,
            .ctx = clientArg->ctx,
        };

        
        if (client.port == 443) {
            // Create SSL object for client
            SSL *ssl = SSL_new(client.ctx);
            SSL_set_fd(ssl, client.sockfd);
            client.ssl = ssl;
            int ssl_err = SSL_accept(ssl);
            if(ssl_err < 0) {
                err("SSL ERR: %s [%d] : %s","SSL/4",ssl_err, get_ssl_err_name(SSL_get_error(ssl,ssl_err)));
                ERR_print_errors_fp(stderr);
                print_net_error(8);
                //Error occurred, log and close down ssl
                // SSL_shutdown(ssl);
                // SSL_free(ssl);
            } 
            // else {
            //     char buffer[128];
            //     SSL_read(ssl, buffer, 128);
            // }
        }
        
        printf("connected ... \n");
        char buf[BUFFER_LEN] = {0};
        int bytes = net_recv(client, buf, sizeof(buf),0);
        // int s=-1,e=0;
        
        request_t req = {0};
        parse_http_request(&req,bytes,buf);

        struct sockaddr_in remote_addr;
        socklen_t addr_len = sizeof(remote_addr);
        getpeername(client.sockfd, (struct sockaddr *)&remote_addr, &addr_len);

        response_t response = NOT_FOUND_RESPONSE;
        
        if (req.path != NULL && strlen(req.path)>0) {
            printf("\n\n------------------\nexists [%s]: %d\n----------------\n",req.path, file_exists(req.path));
            char route_path[1024] = {0};
            sprintf(route_path,"%s:%s",req.method,req.path);
            inf("==> '%s'", route_path);
            int index = hash_map_get(&routes_map, route_path);
            route_t route = global_routes.items[index];
            // printf("\n\n------------------\nexists [%s]: %d\n----------------\n",req.path, file_exists(req.path));
            
            inf(" router [%s]", route.route);
            
            if(strcasecmp(route.route, route_path)==0) {
                if(strcmp(route.route,"GET:/") != 0) 
                    send_headers(200,"Content-Type: text/html",NULL,client);                
                route.handler(req, client);
                
                if (client.port == 443 && client.ssl != NULL) {
                    SSL_shutdown(client.ssl);
                    char buffer[128];
                    SSL_read(client.ssl, buffer, 128);
                    //SSL_free(client.ssl);            
                }
                shutdown(client.sockfd,0x01);
                read(client.sockfd, buf, 128);
                close(client.sockfd);
                
                return NULL;
            }
            else if (file_exists(req.path)) {
                //get_static_file_response(&response,req.path);
                char path[512] = ".";
                strcat(path,req.path);
                serve_static_file(path, client);
                return NULL;
            }
        }


        // if (strcmp(headers, SECRET_TOKEN) == 0)
        // {
        // }
        
        char response_string[2048]; 
        prepare_response_string(response_string,response);
        net_send(client, response_string, strlen(response_string), 0);
        
        if (client.port == 443 && client.ssl != NULL) {
            SSL_shutdown(client.ssl);
            char buffer[128];
            SSL_read(client.ssl, buffer, 128);
            //SSL_free(client.ssl);            
        }
        
        shutdown(client.sockfd,0x01);
        read(client.sockfd, buf, 128);
        close(client.sockfd);
        

        return NULL;
    }

    #ifndef SIGPIPE
    #define	SIGPIPE	13	/* write on a pipe with no one to read it */
    #endif//SIGPIPE
    
    #ifndef SOL_TCP
    #define SOL_TCP 6  /* socket options TCP level */
    #endif//SOL_TCP

    #ifndef TCP_USER_TIMEOUT
    #define TCP_USER_TIMEOUT 18  /* how long for loss retry before timeout [ms] */
    #endif//TCP_USER_TIMEOUT

    HTTP_SERVER_API void start_server(int port, route_list_t *routes) {
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

        socket_t sockfd;
        
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
                print_net_error(0);
            }
        #endif
        
        if((sockfd = socket(AF_INET , SOCK_STREAM , 0 )) == 0) {
            print_net_error(1);
        }

        const char on = 1;
        // const char off = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR , &on, sizeof(on));
        // setsockopt(sockfd, SOL_SOCKET, SO_LINGER    , &on, sizeof(on));
        // setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE , &on, sizeof(on));

        // struct timeval timeout;      
        // timeout.tv_sec = 1;
        // timeout.tv_usec = 0;

        // if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0)
        //     err("[%d] setsockopt failed",1);

        // if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout) < 0)
        //     err("[%d] setsockopt failed",2);   
            
        // int tcp_timeout = 10000;  // user timeout in milliseconds [ms]
        // if (setsockopt (sockfd, SOL_TCP, TCP_USER_TIMEOUT, (char*) &tcp_timeout, sizeof tcp_timeout))           
        //     err("[%d] setsockopt failed",3);   

        if(bind(sockfd, (struct sockaddr*)&address, addrlen) != 0) {
            print_net_error(2);
        }

        printf("server started on port %d !\n", port);
        
        if(listen(sockfd, 10) != 0) {
            print_net_error(3);
        }

        

        printf("listening ... \n");
        socket_t client;

        SSL_CTX *ctx = {0};

        if (port==443) {
            inf("Initialize OpenSSL %p",NULL);
            // Initialize OpenSSL
            SSL_library_init();
            SSL_load_error_strings();
            OpenSSL_add_all_algorithms();

            // Create SSL_CTX
            // ctx = SSL_CTX_new(TLS_server_method());
            ctx = SSL_CTX_new(SSLv23_server_method());
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
                    err("SSL ERR: %s","SSL/3");
                fprintf(stderr, "Private key does not match the certificate\n");
                // Handle error
            }

            
        }

        for(;;){              
            client = accept(sockfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            if (client < 1) {
                print_net_error(9);
                continue;
            }
            client_t clientArg = {client,port,NULL,ctx};
            pthread_t thread;
            if (is_default_handler)
                pthread_create(&thread,NULL,handle_client_connection,(void*)&clientArg);
            else
                pthread_create(&thread,NULL,hsts_handler,(void*)client);
        }
    }
#endif//_SERVER_HTTP_IMPLEMENTATION
#endif//_SERVER_H