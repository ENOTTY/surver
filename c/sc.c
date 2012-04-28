/**
 * @author Vinnie Agriesti (crazychenz@gmail.com)
 */
 
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
 
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <openssl/bio.h> // BIO objects for I/O
#include <openssl/ssl.h> // SSL and SSL_CTX for SSL connections
#include <openssl/err.h> // Error reporting
#include <openssl/sha.h> // Digest functionality

#include "sc.h"

int net_init()
{
#ifdef WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }
#endif
	return 0;
}

int net_buf_simple_http_header(struct net_buf_t * buf, size_t len, char * type)
{
	net_buf_add_cstr(buf, "HTTP/1.0 200 OK\r\n");
	net_buf_add_fmt(buf, "Content-Length: %d\r\n", len);
	net_buf_add_fmt(buf, "Content-Type: %s\r\n", type);
	return 0;
}

int file_load(char * fname, char ** data, size_t * data_len)
{
    int ret = 0;
    FILE * fp = NULL;
    int bytes_read = 0, off = 0;
    char c = 0;
    
    fp = fopen(fname, "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open filename\n");
        ret = -1;
				goto done;
    }
    while (!feof(fp)) {
        bytes_read = fread(&c, 1, 1, fp);
        if (bytes_read >= 0) {
            *data_len += bytes_read;
        }
        else {
            fprintf(stderr, "Failed to count js file.\n");
            ret = -1;
						goto done;
        }
    }
    *data = calloc(1, *data_len + 1);
    if (!*data) {
        fprintf(stderr, "Failed to alloc mem for js.\n");
        ret = -1;
				goto done;
    }
    rewind(fp);
    while (!feof(fp)) {
        bytes_read = fread(*data + off, 1, 1, fp);
        if (bytes_read < 0) {
            fprintf(stderr, "Failed to read js file.\n");
            ret = -1;
						goto done;
        }
        off += 1;
    }

done:
    return ret;
}

int net_buf_sizer(struct net_buf_t * buf)
{
    buf->ptr = NULL;
    buf->len = (size_t)-1;
    buf->off = 0;
		return 0;
}

int net_buf_alloc(struct net_buf_t * buf)
{
    buf->len = buf->off;
    buf->off = 0;
    buf->ptr = calloc(1, buf->len + 1);
    if (!buf->ptr) {
        return -ENOMEM;
    }
    return 0;
}

int net_buf_add_bin(struct net_buf_t * buf, char * str, size_t str_len)
{
    size_t len = buf->len - buf->off;
    len = str_len > len ? len : str_len;
    
    if (buf->ptr) {
        memcpy(buf->ptr + buf->off, str, len);
    }
    
    buf->off += len;
		return 0;
}

int net_buf_add_cstr(struct net_buf_t * buf, char * str)
{
    size_t len = buf->len - buf->off;
    len = strlen(str) > len ? len : strlen(str);
    
    if (buf->ptr) {
        memcpy(buf->ptr + buf->off, str, len);
    }
    
    buf->off += len;
		return 0;
}

int net_buf_add_fmt(struct net_buf_t * buf, char * fmt, ...)
{
    va_list ap;
    int bytes_printed = 0;
    char * ptr = NULL;
    size_t len = 0;
    
    va_start(ap, fmt);
    
    if (buf->ptr) {
       len = buf->len - buf->off;
       ptr = buf->ptr + buf->off;
    }
    
    bytes_printed = vsnprintf(ptr, len, fmt, ap);
    if (bytes_printed > 0) {
        buf->off += bytes_printed;
    }
    
    va_end(ap);
    
    return 0;
}

int net_server_init(struct net_serv_t * serv, short port)
{
    struct sockaddr_in serv_addr;
    
    if (!serv) {
        return -1;
    }
    
    memset(serv, 0, sizeof(struct net_serv_t));
	
	serv->log_fp = fopen("server.log", "ab");
	if (!serv->log_fp) {
		fprintf(stderr, "Failed to open server.log\n");
		return -1;
	}
    
    serv->buf = calloc(1, 0xFFFFF);
    if (!serv->buf) {
        return -1;
    }
    serv->buf_len = 0xFFFFE;
    serv->buf_off = 0;
    
    serv->sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(serv->sock == -1) {
        fprintf(stderr, "Failed to create server socket");
        goto done;
    }
    
    serv->max_sock = serv->sock;

    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(serv->sock, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in))) {
        perror("error bind failed");
        close(serv->sock);
        serv->sock = -1;
        goto done;
    }

    if(listen(serv->sock, 10)) {
        perror("error listen failed");
        close(serv->sock);
        serv->sock = -1;
        goto done;
    }
    
done:
    return serv->sock;
}

void net_ssl_library_init() {
    static int ssl_loaded = 0;
    
    if (!ssl_loaded) {
        CRYPTO_malloc_init();         // Init malloc, free, for OpenSSL's use
        SSL_library_init();           // Init OpenSSL's SSL libraries
        SSL_load_error_strings();     // Load SSL error strings
        ERR_load_BIO_strings();       // Load BIO error strings
        OpenSSL_add_all_algorithms(); // Load all available encrypt algorithms
        ++ssl_loaded;
    }
}

void net_tlsv1_server_init(struct net_serv_t * serv, short port)
{
    net_ssl_library_init();
    
    net_server_init(serv, port);

    serv->ctx = SSL_CTX_new(TLSv1_server_method());
    if (!serv->ctx) {
        return;
    }
    SSL_CTX_set_options(serv->ctx, SSL_OP_SINGLE_DH_USE);
    SSL_CTX_use_certificate_file(serv->ctx, "server.crt", SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(serv->ctx, "server.key", SSL_FILETYPE_PEM);
    
    return;
}

int net_ssl_timeout_read(struct net_serv_t * serv)
{
    int ret = 0;
    fd_set in;
    size_t bytes_read = 0;
    struct timeval timeout;
    int cnt = 0;
    
    if (!serv || serv->max_sock == -1 || serv->sock == -1) {
        return -1;
    }
    
    memset(serv->buf, 0, serv->buf_len);
    serv->buf_off = 0;
    while (1) {
        memset(&timeout, 0, sizeof(struct timeval));
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;
        FD_ZERO(&in);
        FD_SET(serv->client, &in);

        if (cnt > 400) {
            fprintf(stderr, "Someone is slowing us down... kill it!");
            ret = -1;
            goto done;
        }
        
        switch(select(serv->max_sock+1, &in, NULL, NULL, &timeout)) {
            case 0:
                ret = 0;
                goto done;
            case 1:
                bytes_read = 
                    SSL_read(serv->ssl, serv->buf + serv->buf_off, 
                             serv->buf_len - serv->buf_off);
                serv->buf_off += bytes_read;
                cnt += 1;
                break;
            default:
                ret = -1;
                goto done;
        }
    }

done:
    return ret;
}

int net_ssl_accept_client(struct net_serv_t * serv)
{
    // Accept incoming connection
    memset(&serv->client_addr, 0, sizeof(struct sockaddr_in));
    serv->client_addr_len = sizeof(struct sockaddr_in);
    serv->client = accept(serv->sock, (struct sockaddr *)&serv->client_addr, 
                          &serv->client_addr_len);

    // If serv goes bad, we need to restart the server
    if(serv->client < 0) {
        fprintf(stderr, "error accept failed");
        close(serv->sock);
        serv->sock = -1;
        goto done;
    }

    if (serv->client > serv->sock) {
        serv->max_sock = serv->client;
    }
    else {
        serv->max_sock = serv->sock;
    }

    serv->ssl = SSL_new(serv->ctx);
    if (!serv->ssl) {
        fprintf(stderr, "Failed to create SSL object\n");
    }
    SSL_set_fd(serv->ssl, serv->client);
    SSL_accept(serv->ssl);

done:
    return 0;
}
