/**
 * @author Vinnie Agriesti (crazychenz@gmail.com)
 */

#include <stdio.h>

#include "sc.h"

#ifndef REQHDLR_H
#define REQHDLR_H

extern char * form;
extern size_t form_len;
extern char * js;
extern size_t js_len;
extern char * css;
extern size_t css_len;
extern char * xml;
extern size_t xml_len;
extern char * xsl;
extern size_t xsl_len;

int file_write_base64(FILE * fp, char * data, size_t data_len);

int show_http(struct net_serv_t * serv, 
              char * data, size_t data_len, 
              const char * type, int add_cookie);

int process_request(struct net_serv_t * serv);

#endif
