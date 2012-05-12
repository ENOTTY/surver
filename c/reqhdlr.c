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
#include <openssl/evp.h>
#include <openssl/err.h> // Error reporting
#include <openssl/sha.h> // Digest functionality
#include <openssl/ssl.h>

#include "sc.h"
#include "form.h"
#include "reqhdlr.h"

int file_write_base64(FILE * fp, char * data, size_t data_len)
{
	BIO * bio = NULL, * b64 = NULL;
	
	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new_fp(fp, BIO_NOCLOSE);
	bio = BIO_push(b64, bio);
	
	BIO_write(bio, data, data_len);
	//BIO_flush(bio);
	
	BIO_free_all(bio);

	return 0;
}

int show_http(struct net_serv_t * serv, 
              char * data, size_t data_len, int add_cookie)
{
	struct net_buf_t hdr;
	struct net_buf_t body;

	memset(&hdr, 0, sizeof(hdr));
	memset(&body, 0, sizeof(body));
	
	// Build the HTTP Body before headers so we know content length
	net_buf_sizer(&body);
	net_buf_add_bin(&body, data, data_len);
	net_buf_alloc(&body);
	net_buf_add_bin(&body, data, data_len);
	
	// Build the HTTP headers
	net_buf_sizer(&hdr);
	net_buf_simple_http_header(&hdr, body.len, "text/html");
	if (add_cookie) {
		net_buf_cookie_id(&hdr, serv->buf, serv->buf_off);
	}
	net_buf_add_cstr(&hdr, "\r\n");
	net_buf_alloc(&hdr);
	net_buf_simple_http_header(&hdr, body.len, "text/html");
	if (add_cookie) {
		net_buf_cookie_id(&hdr, serv->buf, serv->buf_off);
	}
	net_buf_add_cstr(&hdr, "\r\n");
	
	// Now dump HTTP headers then the body to SSL stream
	SSL_write(serv->ssl, hdr.ptr, hdr.len);
	SSL_write(serv->ssl, body.ptr, body.len);

	return 0;
}

int process_request(struct net_serv_t * serv)
{
	char * content = NULL; // pointer used to track content
	size_t content_len = 0;
	int add_cookie = 0;
	
	if (!memcmp(serv->buf, "GET", 3) && !serv->buf[serv->buf_off] &&
			!strstr(serv->buf, "\r\nCookie: id=")) {
		++add_cookie;
	}
	
	// Process request
	if (!memcmp(serv->buf, "GET / ", 6) || 
			!memcmp(serv->buf, "GET /survey.html ", 17)) {
		show_http(serv, form, form_len, add_cookie);
	}
	
	if (!memcmp(serv->buf, "GET /survey.js ", 15)) {
		show_http(serv, js, js_len, 0);
	}

	if (!memcmp(serv->buf, "GET /survey.css ", 16)) {
		show_http(serv, css, css_len, 0);
	}
	
	if (!serv->buf[serv->buf_off] && !memcmp(serv->buf, "POST ", 5) &&
			strstr(serv->buf, "\r\n\r\n")) {
			
		// Grab a pointer to the content
		content = strstr(serv->buf, "\r\n\r\n") + 4;
		content_len = serv->buf_off - (content - serv->buf);
		
		//process_post(content, content_len);
		
		// Dump content base64 encoded to log file
		if (serv->log_fp) {
			file_write_base64(serv->log_fp, content, content_len);
			fwrite("----\r\n", 1, 6, serv->log_fp);
			fflush(serv->log_fp);
		}
		
		// Dump an empty form
		show_http(serv, form, form_len, add_cookie);
	}
	
	return 0;
}


