#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "node.h"
#include "display.h"

const char *get_last_segment(const char *path)
{
    const char *last_slash = strrchr(path, '/');
    if (last_slash)
    {
        return last_slash + 1;
    }
    else
    {
        return path;
    }
}

void format_size(off_t size, char *buffer, size_t buffer_size)
{
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double size_d = size;

    while (size_d >= 1024 && unit_index < 4)
    {
        size_d /= 1024;
        unit_index++;
    }

    snprintf(buffer, buffer_size, "%.2f %s", size_d, units[unit_index]);
}

int get_total_nodes(DirectoryNode *node)
{
    int count = 0;
    while (node)
    {
        count++;
        node = node->next;
    }
    return count;
}

void reveal_in_finder(const char *path)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        execl("/usr/bin/open", "open", "-R", path, (char *)NULL);
        _exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        waitpid(pid, NULL, 0);
    }
    else
    {
        perror("fork");
    }
}
