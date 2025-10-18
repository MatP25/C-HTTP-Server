#ifndef FILE_HELPERS_H
#define FILE_HELPERS_H


#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

struct file_data {
    int size;
    void *data;
};

char *get_file_mime_type(char *file_name);
struct file_data *load_file(char *filename);
void file_free(struct file_data *filedata);

#endif