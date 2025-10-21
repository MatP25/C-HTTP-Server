#include "other_helpers.h"

void *extract_substr(char *source, char *start_delim, char *end_delim, bool include_start, bool include_end) {

    if (start_delim == NULL || end_delim == NULL || source == NULL) {
        return NULL;
    }

    char *start_ptr = NULL;
    if (strcmp(start_delim, "") == 0) {
        start_ptr = source;
    } else {
        start_ptr = strstr(source, start_delim);
        if (!start_ptr) {
            return NULL;
        }
        if (!include_start) {
            start_ptr += strlen(start_delim);
        }
    }

    char *end_ptr = strstr(start_ptr, end_delim);
    if (!end_ptr) {
        return NULL;
    }

    if (include_end) {
        end_ptr += strlen(end_delim);        
    }

    size_t substr_length = end_ptr - start_ptr;
    char *substr = (char *)malloc(substr_length + 1);
    if (!substr) {
        return NULL;
    }

    memcpy(substr, start_ptr, substr_length);
    substr[substr_length] = '\0';

    return substr;
}