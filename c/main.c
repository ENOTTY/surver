/**
 * @author Vinnie Agriesti (crazychenz@gmail.com)
 */

/*
 * To create a new self-signed server cert...
 * $ openssl genrsa -des3 -out server.key 1024
 * $ openssl req -new -key server.key -out server.csr
 * $ cp server.key server.key.locked
 * $ openssl rsa -in server.key.locked -out server.key
 * $ openssl x509 -req -days 365 -in server.csr -signkey server.key -out server.crt
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
#include "reqhdlr.h"
#include "form.h"
#include "list.h"

// Some private variables we initialize once
char * form = NULL;
size_t form_len = 0;
char * js = NULL;
size_t js_len = 0;
char * css = NULL;
size_t css_len = 0;

int file_load(char * fname, char ** data, size_t * data_len)
{
	int ret = 0;
	FILE * fp = NULL;
	size_t bytes_read = 0, off = 0;
	char c = 0;
    
	fp = fopen(fname, "rb");
	if (!fp) {
		fprintf(stderr, "Failed to open filename\n");
		ret = -1;
		goto done;
	}
	while (!feof(fp)) {
		bytes_read = fread(&c, 1, 1, fp);
		if (ferror(fp) && !feof(fp)) {
			fprintf(stderr, "Failed to count file.\n");
			ret = -1;
			goto done;
		}
		*data_len += bytes_read;
	}
	*data = calloc(1, *data_len + 1);
	if (!*data) {
		fprintf(stderr, "Failed to alloc mem for js.\n");
		ret = -1;
		goto done;
	}

	rewind(fp);

	while (!feof(fp) && off < *data_len) {
		bytes_read = fread(*data + off, 1, 1, fp);
		if (ferror(fp) && !feof(fp)) {
			fprintf(stderr, "Failed to read file.\n");
			ret = -1;
			goto done;
		}
		off += 1;
	}

done:
	return ret;
}

int main()
{
	struct net_serv_t serv;

	memset(&serv, 0, sizeof(serv));
    
	// Startup winsock if needed
	net_init();
	
	// Seed our pseudo RNG
	srand ( time(NULL) );

	// Initialize our static objects
	file_load(SURVEY_HTML, &form, &form_len);
	file_load(SURVEY_JS, &js, &js_len);
	file_load(SURVEY_CSS, &css, &css_len);

	// If server goes belly up, we'll just attempt to re-init it.    
	for (;;) { 

		// Startup a TLS listening server
		if (net_tlsv1_server_init(&serv, SURVEY_PORT)) {
			fprintf(stderr, "Failed to restart service.\n");
			break;
		}

		// For simplicity, we're going to be single threaded, meaning
		// we're only servicing one connection at a time.
		for(;;) {
			if (net_ssl_accept_client(&serv)) {
				fprintf(stderr, "Failed to accept SSL connection\n");
				break; // We'll just restart the server on accept fail
			}
			fprintf(stderr, "Got connection\n");

			if (net_ssl_timeout_read(&serv, 10000)) {
				// read failed, kill connection
				//SSL_write(ssl, "HTTP/1.0 200 OK\r\n"
				//    "Content-Length: 26\r\nContent-Type: text/html\r\n")
				//    "Failed to process request.", 89);
				//SSL_shutdown(ssl);
				//SSL_free(ssl);
				//shutdown(client, SHUT_RDWR)
				//close(client);
				fprintf(stderr, "net_ssl_timeout_read failed()\n");
				continue;
			}

			//fprintf(stderr, "Got request %d\n", (int)serv.buf_off);
			process_request(&serv);

			// We're done with this connection, kill it.
			//SSL_free(ssl);

			// Attempt proper client socket shutdown
			net_close_ssl_client(&serv, 50000);

			// Send FIN from service
			//shutdown(serv.client, SHUT_WR);
			// Give client chance to send FIN
			//ret = net_timeout_read(&serv, 10000);
			// Close socket
			//close(serv.client);
			//serv.client = -1;
		}
	}

	return EXIT_SUCCESS;  
}
