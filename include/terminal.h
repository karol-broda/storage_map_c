#ifndef TERMINAL_H
#define TERMINAL_H

typedef struct
{
    int width;
    int height;
} TermSize;

TermSize get_term_size(void);
void handle_winch(int sig);
void handle_exit();
void cleanup();

#endif // TERMINAL_H
