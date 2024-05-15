#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <pthread.h>
#include <sys/types.h>
#include "node.h"

typedef struct ThreadData
{
    DirectoryNode *parent;
    char *dirPath;
    struct ThreadData *next;
} ThreadData;

void initialize_thread_pool();
void destroy_thread_pool();
DirectoryNode *add_directory_path(DirectoryNode *parent, const char *path, off_t size, int is_file);
off_t traverse_directory(DirectoryNode *parent, char *dirPath);
void *traverse_directory_thread(void *arg);
void delete_directory(DirectoryNode *node);

#endif // DIRECTORY_H
