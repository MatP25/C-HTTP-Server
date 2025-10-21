#ifndef FILE_HELPERS_H
#define FILE_HELPERS_H

#include "includes.h"

struct file_data {
    int size;
    void *data;
};

char *get_file_mime_type(char *file_name);
struct file_data *load_file(char *filename);
void file_free(struct file_data *filedata);
int write_file(char *filename, char *data, size_t data_size);

#endif