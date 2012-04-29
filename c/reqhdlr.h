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

int file_write_base64(FILE * fp, char * data, size_t data_len);

int show_http(struct net_serv_t * serv, 
              char * data, size_t data_len, int add_cookie);

int process_request(struct net_serv_t * serv);

#endif
