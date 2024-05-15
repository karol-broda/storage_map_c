#include <ncurses.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <pthread.h>
#include "terminal.h"
#include "node.h"

extern int terminal_width;
extern int terminal_height;
extern pthread_mutex_t display_mutex;
extern DirectoryList dirList;

TermSize get_term_size(void)
{
    initscr();
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    endwin();
    if (max_y == -1 || max_x == -1)
    {
        fprintf(stderr, "getmaxyx() failed\n");
        TermSize size = {0, 0};
        return size;
    }
    TermSize size = {max_x - 8, max_y - 3};
    return size;
}

void handle_winch(int sig)
{
    (void)sig;

    TermSize size = get_term_size();
    terminal_width = size.width;
    terminal_height = size.height;
    printf("\033[H\033[J");
}

void handle_exit()
{
    cleanup();
    exit(EXIT_FAILURE);
}

void cleanup()
{
    endwin();
    pthread_mutex_destroy(&dirList.mutex);
    pthread_mutex_destroy(&display_mutex);
}
