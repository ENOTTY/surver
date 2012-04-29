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
#include <netinet/tcp.h>
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
        return -EPERM;
    }
#endif
	return 0;
}

int net_buf_cookie_id(struct net_buf_t * buf, char * data, size_t data_len)
{
	SHA_CTX sha_ctx;
	unsigned char digest[21] = {0};
	int rint = 0, i = 0;

	if (!buf || !data || data_len == 0) {
		return -EINVAL;
	}
	
	// create cookie ID
	memset(&sha_ctx, 0, sizeof(sha_ctx));
	rint = rand();
	SHA1_Init(&sha_ctx);
	SHA1_Update(&sha_ctx, data, data_len);
	SHA1_Update(&sha_ctx, &rint, sizeof(rint));
	SHA1_Final(digest, &sha_ctx);
	
	// Make digest ASCII friendly (degrades digest)
	for (i = 0; i < 20; ++i) {
		digest[i] = (digest[i] % 26) + 0x61;
	}
	
	net_buf_add_fmt(buf, "Set-Cookie: id=%s; "
				"Expires=Tue, 15 Jan 2032 21:47:38 GMT; Path=/; "
				"Domain=127.0.0.1; HttpOnly; Secure\r\n", digest);

	return 0;
}

int net_buf_simple_http_header(struct net_buf_t * buf, size_t len, char * type)
{
	if (!buf || !len || !type) {
		return -EINVAL;
	}

	net_buf_add_cstr(buf, "HTTP/1.0 200 OK\r\n");
	net_buf_add_fmt(buf, "Content-Length: %d\r\n", len);
	net_buf_add_fmt(buf, "Content-Type: %s\r\n", type);

	return 0;
}


int net_buf_sizer(struct net_buf_t * buf)
{
	if (!buf) {
		return -EINVAL;
	}

	buf->ptr = NULL;
	buf->len = (size_t)-1;
	buf->off = 0;
	return 0;
}

int net_buf_alloc(struct net_buf_t * buf)
{
	if (!buf) {
		return -EINVAL;
	}

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
    size_t len = 0;

		if (!buf || !str || !str_len) {
			return -EINVAL;
		}

		len = buf->len - buf->off;
    len = str_len > len ? len : str_len;
    
    if (buf->ptr) {
        memcpy(buf->ptr + buf->off, str, len);
    }
    
    buf->off += len;
		return 0;
}

int net_buf_add_cstr(struct net_buf_t * buf, char * str)
{
    size_t len = 0;

		if (!buf || !str) {
			return -EINVAL;
		}

		len = buf->len - buf->off;
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

		if (!buf || !fmt) {
			return -EINVAL;
		}
    
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
	int ret = 0;
	int flag = 1;
	struct sockaddr_in serv_addr;
    
	if (!serv) {
		return -EINVAL;
	}
    
	memset(serv, 0, sizeof(struct net_serv_t));
	
	serv->log_fp = fopen("server.log", "ab");
	if (!serv->log_fp) {
		ret = errno;
		fprintf(stderr, "Failed to open server.log\n");
		goto done;
	}
    
	serv->buf = calloc(1, 0xFFFFF);
	if (!serv->buf) {
		ret = errno;
		goto done;
	}
	serv->buf_len = 0xFFFFE;
	serv->buf_off = 0;
    
	serv->sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(serv->sock == -1) {
		ret = errno;
		fprintf(stderr, "Failed to create server socket");
		goto done;
	}
  
	if (serv->sock > serv->max_sock) {
		serv->max_sock = serv->sock;
	}

	setsockopt(serv->sock, SOL_SOCKET, SO_REUSEADDR, 
	           (char *)&flag, sizeof(flag));

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(serv->sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) {
		ret = errno;
		fprintf(stderr, "bind() failed");
		goto done;
	}

	if(listen(serv->sock, 10)) {
		ret = errno;
		fprintf(stderr, "listen() failed");
		goto done;
	}
    
done:
	if (ret) {
		if (serv->buf) {
			memset(serv->buf, 0, serv->buf_len);
			free(serv->buf);
			serv->buf = NULL;
		}
		if (serv->sock >= 0) {
			close(serv->sock);
			serv->sock = -1;
		}
	}

	return ret*-1;
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

int net_tlsv1_server_init(struct net_serv_t * serv, short port)
{
	int ret = 0;

	if (!serv) {
		return -EINVAL;
	}

	(void) net_ssl_library_init();
    
	ret = net_server_init(serv, port);
	if (ret) {
		fprintf(stderr, "Failed to get service socket.\n");
		goto done;
	}

	serv->ctx = SSL_CTX_new(TLSv1_server_method());
	if (!serv->ctx) {
		ret = EPERM;
		fprintf(stderr, "Failed to create SSL context.\n");
		goto done;
	}

	(void) SSL_CTX_set_options(serv->ctx, SSL_OP_SINGLE_DH_USE);
	if (1 != SSL_CTX_use_certificate_file(serv->ctx, SURVEY_CRT, SSL_FILETYPE_PEM)) {
		ret = EPERM;
		fprintf(stderr, "Failed to load certificate.\n");
		goto done;
	}

	if (1 != SSL_CTX_use_PrivateKey_file(serv->ctx, SURVEY_KEY, SSL_FILETYPE_PEM)) {
		ret = EPERM;
		fprintf(stderr, "Failed to load private key.\n");
		goto done;
	}

done:
	if (ret && serv->ctx) {
		SSL_CTX_free(serv->ctx);
		serv->ctx = NULL;
	}
	return ret*-1;
}

int net_timeout_read(struct net_serv_t * serv, int usec)
{
	int ret = 0;
	fd_set in;
	size_t bytes_read = 0;
	struct timeval timeout;
	int cnt = 0;
    
	if (!serv || serv->max_sock == -1 || serv->sock == -1 || 
	    !serv->buf || serv->client == -1) {
		return -EINVAL;
	}
    
	memset(serv->buf, 0, serv->buf_len);
	serv->buf_off = 0;
	while (1) {
		memset(&timeout, 0, sizeof(timeout));
		timeout.tv_sec = 0;
		timeout.tv_usec = usec;
		FD_ZERO(&in);
		FD_SET(serv->client, &in);

		if (cnt > 400) {
			ret = EPERM;
			fprintf(stderr, "Someone is slowing us down... kill it!");
			goto done;
		}
        
		switch(select(serv->max_sock+1, &in, NULL, NULL, &timeout)) {
			case 0:
				ret = 0;
				goto done;
			case 1:
				bytes_read = 
					read(serv->client, serv->buf + serv->buf_off, 
					     serv->buf_len - serv->buf_off);
				serv->buf_off += bytes_read;
				cnt += 1;
				if (bytes_read == 0) {
					fprintf(stderr, "FIN recieved\n");
					goto done;
				}
				break;
			default:
				ret = EIO;
				goto done;
		}
	}

done:
    return ret*-1;
}

int net_ssl_timeout_read(struct net_serv_t * serv, int usec)
{
	int ret = 0;
	fd_set in;
	size_t bytes_read = 0;
	struct timeval timeout;
	int cnt = 0;
    
	if (!serv || serv->max_sock == -1 || serv->sock == -1 || 
	    !serv->buf || serv->client == -1) {
		return -EINVAL;
	}
    
	memset(serv->buf, 0, serv->buf_len);
	serv->buf_off = 0;
	while (1) {
		memset(&timeout, 0, sizeof(timeout));
		timeout.tv_sec = 0;
		timeout.tv_usec = usec;
		FD_ZERO(&in);
		FD_SET(serv->client, &in);

		if (cnt > 400) {
			ret = EPERM;
			fprintf(stderr, "Someone is slowing us down... kill it!");
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
				ret = EIO;
				goto done;
		}
	}

done:
    return ret*-1;
}

int net_ssl_accept_client(struct net_serv_t * serv)
{
	int ret = 0;
	int flag = 1;

	if (!serv) {
		return -EINVAL;
	}

	// Accept incoming connection
	memset(&serv->client_addr, 0, sizeof(serv->client_addr));
	serv->client_addr_len = sizeof(serv->client_addr);
	serv->client = accept(serv->sock, (struct sockaddr *)&serv->client_addr, 
  	                    &serv->client_addr_len);

	// Setup client connection for nodelay (prevents premature resets)
	setsockopt(serv->client, IPPROTO_TCP, TCP_NODELAY, 
	           (char *)&flag, sizeof(flag));

	// If serv goes bad, we need to restart the server
	if(serv->client < 0) {
		ret = errno;
		fprintf(stderr, "accept() failed\n");
		goto done;
	}

	// Recalculate max_sock
	if (serv->client > serv->sock) {
		serv->max_sock = serv->client;
	}
	else {
		serv->max_sock = serv->sock;
	}

	// Do SSL Handshake stuff
	serv->ssl = SSL_new(serv->ctx);
	if (!serv->ssl) {
		ret = EPERM;
		fprintf(stderr, "Failed to create SSL object\n");
		goto done;
	}
	if (1 != SSL_set_fd(serv->ssl, serv->client)) {
		ret = EPERM;
		fprintf(stderr, "Failed to bind client socket to ssl socket\n");
		goto done;
	}
	if (1 != SSL_accept(serv->ssl)) {
		ret = EPERM;
		fprintf(stderr, "Failed to establish SSL connection\n");
		goto done;
	}

done:

	if (ret) {
		close(serv->client);
		serv->client = -1;
		if (serv->ssl) {
			SSL_free(serv->ssl);
			serv->ssl = NULL;
		}
		memset(&serv->client_addr, 0, sizeof(serv->client_addr));
		serv->client_addr_len = 0;
	}

	return ret*-1;
}

int net_close_ssl_client(struct net_serv_t * serv, int usec)
{
	int ret = 0;

	if (!serv || serv->client == -1) {
		return -EINVAL;
	}

	// Do SSL shutdown procedure
	if (serv->ssl) {
		if (SSL_shutdown(serv->ssl) == 2) {
			SSL_shutdown(serv->ssl);
		}
		SSL_free(serv->ssl);
		serv->ssl = NULL;
	}

	// Send FIN from service
	if (shutdown(serv->client, SHUT_WR)) {
		ret = errno;
		goto done;
	}

	// Give client chance to send FIN
	ret = net_timeout_read(serv, usec);

done:

	// Close socket
	close(serv->client);
	serv->client = -1;
	return ret*-1;
}
