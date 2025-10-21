#include "file_helpers.h"

char *get_file_mime_type(char *file_name)
{
    char *ext = strrchr(file_name, '.');

    if (ext == NULL) {
        return MIME_OCTET_STREAM;
    }
    
    ext++;
    // strlower(ext);

    if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0) { return MIME_TEXT_HTML; }
    if (strcmp(ext, "jpeg") == 0 || strcmp(ext, "jpg") == 0) { return MIME_JPEG; }
    if (strcmp(ext, "css") == 0) { return MIME_TEXT_CSS; }
    if (strcmp(ext, "js") == 0) { return MIME_TEXT_JS; }
    if (strcmp(ext, "json") == 0) { return MIME_JSON; }
    if (strcmp(ext, "txt") == 0) { return MIME_TEXT_PLAIN; }
    if (strcmp(ext, "png") == 0) { return MIME_PNG; }

    return MIME_OCTET_STREAM;
}

int write_file(char *filename, char *data, size_t data_size) {
    printf("Writing %zu bytes to file '%s'\n", data_size, filename);
    // Open the file for writing (create if it doesn't exist)
    int file_fd = open(filename, O_APPEND | O_CREAT | O_WRONLY, 0644);

    if (file_fd == -1) {
        perror("Could not open file for writing");
        return -1;
    }

    // Write the data to the file
    ssize_t bytes_written = write(file_fd, data, data_size);
    printf("Bytes written: %zd\n", bytes_written);
    close(file_fd);

    if (bytes_written == -1 || (size_t)bytes_written != data_size) {
        perror("Failed to write file");
        return -1;
    }

    return 0; // Success
}

struct file_data *load_file(char *filename) {
    // Try to open the file as read-only and get the file descriptor
    int file_fd = open(filename, O_RDONLY);
    if (file_fd == -1) {
        return NULL;
    }

    // Get the file size using fstat
    struct stat file_stat;
    fstat(file_fd, &file_stat);
    off_t file_size = file_stat.st_size;

    // Allocate buffer to hold the file content plus null terminator
    size_t buffer_size = file_size + 1;
    char *file_buffer = malloc(buffer_size);
    if (file_buffer == NULL) {
        close(file_fd);
        free(file_buffer);
        return NULL;
    }

    // Read the contents of the file into the buffer
    ssize_t bytes_read = read(file_fd, file_buffer, file_size);
    close(file_fd);
    if (bytes_read == -1) {
        free(file_buffer);
        return NULL;
    }

    // Add null terminator to the end of the buffer
    file_buffer[buffer_size - 1] = '\0';

    // Allocate struct
    struct file_data *filedata = malloc(sizeof(struct file_data));
    if (filedata == NULL) {
        free(file_buffer);
        return NULL;
    }

    // Copy file contents into struct
    char *file_contents = malloc(buffer_size);
    memcpy(file_contents, file_buffer, buffer_size);
    free(file_buffer);

    // Populate struct
    filedata->size = buffer_size;
    filedata->data = file_contents;

    if (filedata->data == NULL) {
        free(filedata);
        return NULL;
    }

    return filedata;
}

void file_free(struct file_data *filedata)
{
    free(filedata->data);
    free(filedata);
}