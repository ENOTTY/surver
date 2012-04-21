#ifndef FORM_H
#define FORM_H

#include "list.h"

struct instructor_t {
	int id;

	char * name;
	size_t name_len;
	
	// Ratings
	int knowledge;
	int preperation; // spelling?
	int communication;
	
	// Instructor Comment
	char * comment;
	size_t comment_len;
	
	struct list_head list;
};

struct form_sub_t {
	// Unique Transaction ID
	char * id;
	size_t id_len;
	
	// Course Name
	char * course;
	size_t course_len;
	
	// Ratings 1 - 5
	int content;
	int labs;
	int organization;
	
	// Course Comment
	char * comment;
	size_t comment_len;
	
	// Instructor List
	struct instructor_t instructor;
	
	// Optional POC
	char * name;
	size_t name_len;
	char * sid;
	size_t sid_len;
	
	struct list_head list;
};

#endif