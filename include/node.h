#ifndef NODE_H
#define NODE_H

#include <pthread.h>

#define MAX_NAME_LENGTH 30

typedef struct DirectoryNode
{
    char path[1024];
    off_t size;
    struct DirectoryNode *next;
    struct DirectoryNode *subdirs;
    struct DirectoryNode *files;
    struct DirectoryNode *parent;
} DirectoryNode;

typedef struct
{
    DirectoryNode *head;
    pthread_mutex_t mutex;
} DirectoryList;

typedef struct UndoInfo
{
    DirectoryNode *node;
    int is_file;
    DirectoryNode *parent;
} UndoInfo;

const char *get_last_segment(const char *path);
void format_size(off_t size, char *buffer, size_t buffer_size);
int get_total_nodes(DirectoryNode *node);
void reveal_in_finder(const char *path);

#endif // NODE_H
