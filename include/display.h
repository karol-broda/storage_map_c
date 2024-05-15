#ifndef DISPLAY_H
#define DISPLAY_H

#include <ncurses.h>
#include "node.h"
#include "directory.h"

extern pthread_mutex_t display_mutex;
extern int terminal_width;
extern int terminal_height;
extern int show_files;
extern UndoInfo undo_info;
extern int loading_done;

void display_directories(DirectoryNode *current, int start_index, int selected_index, int page, int total_pages);
void navigate_directory(DirectoryNode *root);
void show_loading_screen(DirectoryNode *root, const char *startDir);
void clear_loading_screen();
void remove_node_from_list(DirectoryNode **head, DirectoryNode *node_to_remove);
void show_help();
void show_debug(const char *debugging);

#endif // DISPLAY_H
