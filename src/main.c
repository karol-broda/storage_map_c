#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <ncurses.h>
#include "directory.h"
#include "display.h"
#include "terminal.h"

DirectoryList dirList = {NULL, PTHREAD_MUTEX_INITIALIZER};
pthread_mutex_t display_mutex = PTHREAD_MUTEX_INITIALIZER;
int terminal_width = 80;
int terminal_height = 24;
int show_files = 0;
UndoInfo undo_info = {NULL, 0, NULL};

int main(int argc, char *argv[])
{
    if (getuid() != 0)
    {
        fprintf(stderr, "needs to be ran as root\n");
        exit(1);
    }

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    signal(SIGWINCH, handle_winch);
    signal(SIGINT, handle_exit);
    signal(SIGTERM, handle_exit);
    TermSize size = get_term_size();
    terminal_width = size.width;
    terminal_height = size.height;

    char *startDir = strdup(argv[1]);
    DirectoryNode *root = add_directory_path(NULL, startDir, 0, 0);

    initscr();
    keypad(stdscr, TRUE);
    noecho();
    cbreak();

    show_loading_screen(root, startDir);

    navigate_directory(root);

    endwin();

    DirectoryNode *node = root;
    while (node)
    {
        DirectoryNode *tmp = node;
        node = node->next;
        free(tmp);
    }

    pthread_mutex_destroy(&dirList.mutex);
    pthread_mutex_destroy(&display_mutex);
    return 0;
}
