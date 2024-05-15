#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include "directory.h"
#include <stdlib.h>

DirectoryNode *add_directory_path(DirectoryNode *parent, const char *path, off_t size, int is_file)
{
    DirectoryNode *node = (DirectoryNode *)malloc(sizeof(DirectoryNode));
    strncpy(node->path, path, sizeof(node->path) - 1);
    node->path[sizeof(node->path) - 1] = '\0';
    node->size = size;
    node->next = NULL;
    node->subdirs = NULL;
    node->files = NULL;
    node->parent = parent;

    if (is_file)
    {
        if (parent)
        {
            node->next = parent->files;
            parent->files = node;
        }
    }
    else
    {
        if (parent)
        {
            node->next = parent->subdirs;
            parent->subdirs = node;
        }
    }

    return node;
}

void *traverse_directory_thread(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    DirectoryNode *parent = data->parent;
    char *dirPath = data->dirPath;

    DIR *dir = opendir(dirPath);
    if (dir == NULL)
    {
        if (errno == EACCES)
        {
            fprintf(stderr, "opendir: Permission denied: %s\n", dirPath);
        }
        else
        {
            perror("opendir");
        }
        free(dirPath);
        free(data);
        return NULL;
    }

    struct dirent *entry;
    struct stat statbuf;
    off_t total_size = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dirPath, entry->d_name);

        if (lstat(path, &statbuf) == -1)
        {
            if (errno == EACCES)
            {
                fprintf(stderr, "lstat: Permission denied: %s\n", path);
            }
            else
            {
                perror("lstat");
            }
            continue;
        }

        if (S_ISDIR(statbuf.st_mode))
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }
            char *subdirPath = strdup(path);
            DirectoryNode *subdirNode = add_directory_path(parent, subdirPath, 0, 0);

            ThreadData *newData = (ThreadData *)malloc(sizeof(ThreadData));
            newData->parent = subdirNode;
            newData->dirPath = subdirPath;

            pthread_t thread;
            pthread_create(&thread, NULL, traverse_directory_thread, newData);
            pthread_join(thread, NULL);

            total_size += subdirNode->size;
        }
        else
        {
            DirectoryNode *fileNode = add_directory_path(parent, path, statbuf.st_size, 1);
            total_size += fileNode->size;
        }
    }

    closedir(dir);
    free(dirPath);
    parent->size = total_size;
    free(data);
    return NULL;
}

off_t traverse_directory(DirectoryNode *parent, char *dirPath)
{
    ThreadData *data = (ThreadData *)malloc(sizeof(ThreadData));
    data->parent = parent;
    data->dirPath = dirPath;

    pthread_t thread;
    pthread_create(&thread, NULL, traverse_directory_thread, data);
    pthread_join(thread, NULL);

    return parent->size;
}

void delete_directory(DirectoryNode *node)
{
    if (node == NULL)
        return;

    DirectoryNode *subdir = node->subdirs;
    while (subdir)
    {
        DirectoryNode *next_subdir = subdir->next;
        delete_directory(subdir);
        subdir = next_subdir;
    }

    DirectoryNode *file = node->files;
    while (file)
    {
        DirectoryNode *next_file = file->next;
        if (remove(file->path) == -1)
        {
            perror("remove");
        }
        free(file);
        file = next_file;
    }

    if (rmdir(node->path) == -1 && errno != ENOTEMPTY)
    {
        perror("rmdir");
    }

    free(node);
}
