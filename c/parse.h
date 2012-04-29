#include <stdio.h>

#include "form.h"

int duplicate_value(char ** dest, size_t * dest_len, 
                    char * data, size_t data_len);

int duplicate_int_value(int * val, char * data, size_t data_len);

struct instructor_t * 
get_or_create_instructor(struct form_sub_t * form, int id);

int process_attrval(char * data, size_t data_len, struct form_sub_t * form);

void dump_form(FILE * fp, struct form_sub_t * form);

int process_post(char * data, size_t data_len);

