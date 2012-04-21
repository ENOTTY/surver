/**
 * @author Vinnie Agriesti (crazychenz@gmail.com)
 */
 
#include <stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#ifndef NET_LIB_H
#define NET_LIB_H

struct net_buf_t {
    char * ptr;
    size_t len;
    size_t off;
};

struct net_serv_t {
    SSL * ssl;
    SSL_CTX * ctx;
    
    int sock;
    
    
    int client;
    struct sockaddr_in client_addr;
    int client_addr_len;
    
    int max_sock;      // for select
    
    char * buf;
    size_t buf_len, buf_off;
	
	FILE * log_fp;
};

int net_buf_simple_http_header(struct net_buf_t * buf, size_t len, char * type);

int file_load(char * fname, char ** data, size_t * data_len);

int net_buf_sizer(struct net_buf_t * buf);

int net_buf_alloc(struct net_buf_t * buf);

int net_buf_add_bin(struct net_buf_t * buf, char * str, size_t str_len);

int net_buf_add_fmt(struct net_buf_t * buf, char * fmt, ...);

int net_buf_add_cstr(struct net_buf_t * buf, char * str);

int net_server_init(struct net_serv_t * serv, short port);

void net_ssl_library_init();

void net_tlsv1_server_init(struct net_serv_t * serv, short port);

int net_ssl_timeout_read(struct net_serv_t * serv);

int net_ssl_accept_client(struct net_serv_t * serv);

#endif