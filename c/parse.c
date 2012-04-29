#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "form.h"
#include "list.h"

int duplicate_value(char ** dest, size_t * dest_len, 
                    char * data, size_t data_len)
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

struct instructor_t * 
get_or_create_instructor(struct form_sub_t * form, int id)
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
