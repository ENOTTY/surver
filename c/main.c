/**
 * @author Vinnie Agriesti (crazychenz@gmail.com)
 */
 
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <openssl/bio.h> // BIO objects for I/O
#include <openssl/ssl.h> // SSL and SSL_CTX for SSL connections
#include <openssl/err.h> // Error reporting
#include <openssl/sha.h> // Digest functionality

#include "sc.h"
#include "form.h"
#include "list.h"

// Some private variables we initialize once
static char * form = NULL;
static size_t form_len = 0;
static char * js = NULL;
static size_t js_len = 0;

int duplicate_value(char ** dest, size_t * dest_len, char * data, size_t data_len)
{
	char * val = NULL;
	size_t val_len = 0;
	
	val = strchr(data, '=');
	if (!val) {
		return -1;
	}
	
	val_len = data_len - (val - data + 1);
	if (val_len) {
		++val; // skip over equal sign
		*dest = calloc(1, val_len + 1);
		if (!*dest) {
			fprintf(stderr, "Failed to allocate mem for comment.\n");
			exit(1);
		}
		memcpy(*dest, val, val_len);
		*dest_len = val_len;
	}
	
	return 0;
}

int duplicate_int_value(int * val, char * data, size_t data_len)
{
	char * str = NULL;
	size_t str_len = 0;
	
	duplicate_value(&str, &str_len, data, data_len);
	*val = strtoul(str, NULL, 10);
	
	free(str);
	str = NULL;
	str_len = 0;
	
	return 0;
}

struct instructor_t * get_or_create_instructor(struct form_sub_t * form, int id)
{
	struct instructor_t * entry = NULL;
	
	
		list_for_each_entry(entry, &form->instructor.list, list) {
			if (entry->id == id) {
				return entry;
			}
		}

	
	// I guess we didn't find one, we'll create one
	entry = calloc(1, sizeof(struct instructor_t));
	if (!entry) {
		return NULL;
	}
	entry->id = id;
	INIT_LIST_HEAD(&entry->list);
	list_add(&entry->list, &form->instructor.list);
	
	// debug only
	fprintf(stderr, "ID %d ", id);
	
	return entry;
}

int process_attrval(char * data, size_t data_len, struct form_sub_t * form)
{
	char * val = NULL;
	size_t val_len = 0;
	int id = 0;
	struct instructor_t * instructor = NULL;

	//fprintf(stderr, "process_attrval()\n");
	
	
	// Course stuff
	if (!memcmp(data, "course-comment", 14)) {
		duplicate_value(&form->comment, &form->comment_len, data, data_len);
		goto done;
	}
	if (!memcmp(data, "course", 6)) {
		duplicate_value(&form->course, &form->course_len, data, data_len);
		goto done;
	}
	if (!memcmp(data, "cont", 4)) {	
		duplicate_int_value(&form->content, data, data_len);
		goto done;
	}
	if (!memcmp(data, "labs", 4)) {
		duplicate_int_value(&form->labs, data, data_len);
		goto done;
	}
	if (!memcmp(data, "org", 3)) {
		duplicate_int_value(&form->organization, data, data_len);
		goto done;
	}
	if (!memcmp(data, "name", 4)) {
		duplicate_value(&form->name, &form->name_len, data, data_len);
		goto done;
	}
	if (!memcmp(data, "sid", 3)) {
		duplicate_value(&form->sid, &form->sid_len, data, data_len);
		goto done;
	}
	
	// Instructor stuff
	if (!memcmp(data, "instname-", 9)) {
		
		id = strtoul(data + 9, NULL, 10);
		instructor = get_or_create_instructor(form, id);
		if (!instructor) {
			return -1;
		}
		
		duplicate_value(&instructor->name, &instructor->name_len, data, data_len);
		goto done;
	}
	if (!memcmp(data, "know", 4)) {
	
		id = strtoul(data + 4, NULL, 10);
		instructor = get_or_create_instructor(form, id);
		if (!instructor) {
			return -1;
		}
		
		duplicate_int_value(&instructor->knowledge, data, data_len);
		goto done;
	}
	if (!memcmp(data, "prep", 4)) {
		id = strtoul(data + 4, NULL, 10);
		instructor = get_or_create_instructor(form, id);
		if (!instructor) {
			return -1;
		}
		
		duplicate_int_value(&instructor->preperation, data, data_len);
		goto done;
	}
	if (!memcmp(data, "comm", 4)) {
		id = strtoul(data + 4, NULL, 10);
		instructor = get_or_create_instructor(form, id);
		if (!instructor) {
			return -1;
		}
		
		duplicate_int_value(&instructor->communication, data, data_len);
		goto done;
	}
	if (!memcmp(data, "instructor-comment-", 19)) {
		id = strtoul(data + 19, NULL, 10);
		instructor = get_or_create_instructor(form, id);
		if (!instructor) {
			return -1;
		}
		
		duplicate_value(&instructor->comment, &instructor->comment_len, data, data_len);
		goto done;
	}
done:
	fprintf(stderr, "\t%s\n", data);
	return 0;
}

void dump_form(FILE * fp, struct form_sub_t * form)
{
	struct instructor_t * entry = NULL;
	
	fprintf(fp, "--- Course Feedback ---\n");
	if (form->course) fprintf(fp, "Course: %s\n", form->course);
	fprintf(fp, "* Course Ratings (1 - Bad, 5 - Good)\n");
	if (form->content) fprintf(fp, "Content: %d\n", form->content);
	if (form->labs) fprintf(fp, "Labs: %d\n", form->labs);
	if (form->organization) fprintf(fp, "Organization: %d\n", form->organization);
	if (form->comment) fprintf(fp, "Comment: %s\n", form->comment);
	
	list_for_each_entry(entry, &form->instructor.list, list) {
		fprintf(fp, "--- Instructor Feedback ---\n");
		if (entry->name) fprintf(fp, "Name: %s\n", entry->name);
		fprintf(fp, "* Instructor Skill Ratings (1 - Bad, 5 - Good)\n");
		if (entry->knowledge) fprintf(fp, "Knowledge: %d\n", entry->knowledge);
		if (entry->preperation) fprintf(fp, "Preperation: %d\n", entry->preperation);
		if (entry->communication) fprintf(fp, "Communication: %d\n", entry->communication); 
		if (entry->comment) fprintf(fp, "Comment: %s\n", entry->comment);
	}
	
	fprintf(fp, "--- (Optional) Point Of Contact ---\n");
	if (form->name) fprintf(fp, "Name: %s\n", form->name);
	if (form->sid) fprintf(fp, "SID: %s\n", form->sid);
	
	return;
}

int process_post(char * data, size_t data_len)
{
	char buf[0x10000] = {0};
	size_t data_off = 0;
	char * ptr = NULL;
	
	// Ensure data is semi-safe for ANSI-C
	if (data[data_len]) {
		// not null terminated
		return -1;
	}
	
	// Setup the form object
	struct form_sub_t * form = calloc(1, sizeof(struct form_sub_t));
	if (!form) {
		return -1;
	}
	INIT_LIST_HEAD(&form->list);
	INIT_LIST_HEAD(&form->instructor.list);
	//list_add(&form->list, &form_list->list);
	
	fprintf(stderr, "Content: %s\n", data);
	
	// Tokenize and process each attribute value pair.
	do {
		ptr = strchr(data + data_off, '&');
		if (!ptr) {
			ptr = data + data_len;
		}
		
		memcpy(buf, data + data_off, ptr - (data + data_off));
		process_attrval(buf, ptr - (data + data_off), form);
		data_off += ptr - (data + data_off) + 1;
		memset(buf, 0, 0x10000);
	} while (ptr != data + data_len && data_off < data_len);
	
	// Dump out the form
	dump_form(stderr, form);
	
	return 0;
}

int net_buf_cookie_id(struct net_buf_t * buf, char * data, size_t data_len)
{
	SHA_CTX sha_ctx = {0};
    unsigned char digest[21] = {0};
	int rint = 0, i = 0;
	
	// create cookie ID
	rint = rand();
	SHA1_Init(&sha_ctx);
	SHA1_Update(&sha_ctx, data, data_len);
	SHA1_Update(&sha_ctx, &rint, sizeof(int));
	SHA1_Final(digest, &sha_ctx);
	
	// Make digest ASCII friendly (degrades digest)
	for (i = 0; i < 20; ++i) {
		digest[i] = (digest[i] % 26) + 0x61;
	}
	
	net_buf_add_fmt(buf, "Set-Cookie: id=%s; "
				"Expires=Tue, 15 Jan 2032 21:47:38 GMT; Path=/; "
				"Domain=127.0.0.1; HttpOnly; Secure\r\n", digest);
}

int file_write_base64(FILE * fp, char * data, size_t data_len)
{
	BIO * bio = NULL, * b64 = NULL;
	
	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new_fp(fp, BIO_NOCLOSE);
	bio = BIO_push(b64, bio);
	
	BIO_write(bio, data, data_len);
	BIO_flush(bio);
	
	BIO_free_all(bio);

	return 0;
}

int show_form(struct net_serv_t * serv, int add_cookie)
{
	struct net_buf_t hdr = {0};
    struct net_buf_t body = {0};
	
	// Build the HTTP Body before headers so we know content length
	net_buf_sizer(&body);
	net_buf_add_bin(&body, form, form_len);
	net_buf_alloc(&body);
	net_buf_add_bin(&body, form, form_len);
	
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
}

int show_js(struct net_serv_t * serv)
{
	struct net_buf_t hdr = {0};
    struct net_buf_t body = {0};
	
	// Build the HTTP Body before headers so we know content length
	net_buf_sizer(&body);
	net_buf_add_bin(&body, js, js_len);
	net_buf_alloc(&body);
	net_buf_add_bin(&body, js, js_len);
	
	// Build the HTTP headers
	net_buf_sizer(&hdr);
	net_buf_simple_http_header(&hdr, body.len, "text/javascript");
	net_buf_add_cstr(&hdr, "\r\n");
	net_buf_alloc(&hdr);
	net_buf_simple_http_header(&hdr, body.len, "text/javascript");
	net_buf_add_cstr(&hdr, "\r\n");
	
	// Now dump HTTP headers then the body to SSL stream
	SSL_write(serv->ssl, hdr.ptr, hdr.len);
	SSL_write(serv->ssl, body.ptr, body.len);
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
		show_form(serv, add_cookie);
	}
	
	if (!memcmp(serv->buf, "GET /survey.js ", 15)) {
		show_js(serv);
	}
	
	if (!serv->buf[serv->buf_off] && !memcmp(serv->buf, "POST ", 5) &&
			strstr(serv->buf, "\r\n\r\n")) {
			
		// Grab a pointer to the content
		content = strstr(serv->buf, "\r\n\r\n") + 4;
		content_len = serv->buf_off - (content - serv->buf);
		
		process_post(content, content_len);
		
		// Dump content base64 encoded to log file
		if (serv->log_fp) {
			fwrite("----\r\n", 1, 6, serv->log_fp);
			file_write_base64(serv->log_fp, content, content_len);
			fwrite("\r\n", 1, 2, serv->log_fp);
			fflush(serv->log_fp);
		}
		
		// Dump an empty form
		show_form(serv, add_cookie);
	}
	
	return 0;
}

int main(void)
{
    struct net_serv_t serv = {0};
    
	// Seed our pseudo RNG
	srand ( time(NULL) );

	// Initialize our static objects
    file_load("survey.html", &form, &form_len);
    file_load("survey.js", &js, &js_len);
	
	// If server goes belly up, we'll just attempt to re-init it.    
    for (;;) { 

        // Startup a TLS listening server
        net_tlsv1_server_init(&serv, 4343);

		// For simplicity, we're going to be single threaded, meaning
		// we're only servicing one connection at a time.
        for(;;)
        {
            net_ssl_accept_client(&serv);
            fprintf(stderr, "Got connection\n");

            if (net_ssl_timeout_read(&serv)) {
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
            fprintf(stderr, "Got request %d\n", serv.buf_off);
			
			process_request(&serv);

            // We're done with this connection, kill it.
            //SSL_shutdown(ssl);
            //SSL_free(ssl);
            //shutdown(client, SHUT_RDWR);
            //close(client);
            //client = -1;
        }
    }
done:
    return EXIT_SUCCESS;  
}
