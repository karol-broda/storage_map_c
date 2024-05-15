#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "display.h"

int loading_done = 0;

void *loading_animation(void *arg)
{
    (void)arg;
    const char *loading_msgs[] = {
        "Loading, please wait.   ",
        "Loading, please wait..  ",
        "Loading, please wait... ",
        "Loading, please wait...."};
    int msg_count = sizeof(loading_msgs) / sizeof(loading_msgs[0]);
    int msg_index = 0;

    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);

    while (!loading_done)
    {
        attron(COLOR_PAIR(1));
        clear();
        mvprintw(terminal_height / 2 - 1, (terminal_width - strlen(loading_msgs[msg_index])) / 2, "%s", loading_msgs[msg_index]);

        int bar_width = terminal_width / 2;
        int progress = (msg_index + 1) * (bar_width / msg_count);

        mvprintw(terminal_height / 2 + 1, (terminal_width - bar_width) / 2, "[");
        for (int i = 0; i < bar_width; i++)
        {
            if (i < progress)
                mvprintw(terminal_height / 2 + 1, (terminal_width - bar_width) / 2 + 1 + i, "=");
            else
                mvprintw(terminal_height / 2 + 1, (terminal_width - bar_width) / 2 + 1 + i, " ");
        }
        mvprintw(terminal_height / 2 + 1, (terminal_width + bar_width) / 2, "]");

        refresh();
        attroff(COLOR_PAIR(1));

        msg_index = (msg_index + 1) % msg_count;
        usleep(250000);
    }

    return NULL;
}

void show_loading_screen(DirectoryNode *root, const char *startDir)
{
    pthread_t loading_thread;
    loading_done = 0;
    pthread_create(&loading_thread, NULL, loading_animation, NULL);

    root->size = traverse_directory(root, (char *)startDir);

    loading_done = 1;
    pthread_join(loading_thread, NULL);

    clear_loading_screen();
}

void clear_loading_screen()
{
    clear();
    refresh();
}

void display_directories(DirectoryNode *current, int start_index, int selected_index, int page, int total_pages)
{
    pthread_mutex_lock(&display_mutex);
    clear();
    mvprintw(0, 0, "Directory: %s", current->path);
    DirectoryNode *node = current->subdirs;
    int index = 1;
    off_t total_size = current->size;

    int count = 0;
    while (node && count < terminal_height - 4)
    {
        if (index >= start_index)
        {
            double percentage = (total_size > 0) ? (double)node->size / total_size * 100 : 0;
            const char *lastSegment = get_last_segment(node->path);
            char displayName[MAX_NAME_LENGTH + 1];
            if (strlen(lastSegment) > MAX_NAME_LENGTH)
            {
                strncpy(displayName, lastSegment, MAX_NAME_LENGTH - 3);
                strcpy(displayName + MAX_NAME_LENGTH - 3, "...");
            }
            else
            {
                strncpy(displayName, lastSegment, MAX_NAME_LENGTH);
                displayName[MAX_NAME_LENGTH] = '\0';
            }

            int available_space = terminal_width - MAX_NAME_LENGTH - 22;
            int barLength = (percentage / 100) * available_space;

            char size_str[20];
            format_size(node->size, size_str, sizeof(size_str));

            if (index == selected_index)
            {
                attron(A_REVERSE);
            }
            mvprintw(count + 1, 0, "%2d. %-*s [", index, MAX_NAME_LENGTH, displayName);
            for (int i = 0; i < available_space; ++i)
            {
                if (i < barLength)
                {
                    if (i == barLength - 1)
                    {
                        printw(">");
                    }
                    else
                    {
                        printw("-");
                    }
                }
                else
                {
                    printw(" ");
                }
            }
            printw("] %6.2f%%, %10s", percentage, size_str);
            if (index == selected_index)
            {
                attroff(A_REVERSE);
            }
            count++;
        }
        node = node->next;
        index++;
    }

    if (show_files)
    {
        node = current->files;
        while (node && count < terminal_height - 4)
        {
            if (index >= start_index)
            {
                double percentage = (total_size > 0) ? (double)node->size / total_size * 100 : 0;
                const char *lastSegment = get_last_segment(node->path);
                char displayName[MAX_NAME_LENGTH + 1];
                if (strlen(lastSegment) > MAX_NAME_LENGTH)
                {
                    strncpy(displayName, lastSegment, MAX_NAME_LENGTH - 3);
                    strcpy(displayName + MAX_NAME_LENGTH - 3, "...");
                }
                else
                {
                    strncpy(displayName, lastSegment, MAX_NAME_LENGTH);
                    displayName[MAX_NAME_LENGTH] = '\0';
                }

                int available_space = terminal_width - MAX_NAME_LENGTH - 22;
                int barLength = (percentage / 100) * available_space;

                char size_str[20];
                format_size(node->size, size_str, sizeof(size_str));

                if (index == selected_index)
                {
                    attron(A_REVERSE);
                }
                mvprintw(count + 1, 0, "%2d. %-*s [", index, MAX_NAME_LENGTH, displayName);
                for (int i = 0; i < available_space; ++i)
                {
                    if (i < barLength)
                    {

                        if (i == barLength - 1)
                        {
                            printw(">");
                        }
                        else
                        {
                            printw("-");
                        }
                    }
                    else
                    {
                        printw(" ");
                    }
                }
                printw("] %6.2f%%, %10s", percentage, size_str);
                if (index == selected_index)
                {
                    attroff(A_REVERSE);
                }
                count++;
            }
            node = node->next;
            index++;
        }
    }

    if (total_pages > 1)
    {
        mvprintw(terminal_height - 1, 0, "Page %d of %d - Use arrow keys to navigate, Enter to select, 'b' to go back, 'q' to quit, 't' to toggle files, 'h' for help", page, total_pages);
    }
    else
    {
        mvprintw(terminal_height - 1, 0, "Use arrow keys to navigate, Enter to select, 'b' to go back, 'q' to quit, 't' to toggle files, 'h' for help");
    }

    refresh();
    pthread_mutex_unlock(&display_mutex);
}

DirectoryNode *get_node_at_index(DirectoryNode *current, int index, int include_files)
{
    DirectoryNode *node = current->subdirs;
    int count = 1;

    while (node)
    {
        if (count == index)
            return node;
        node = node->next;
        count++;
    }

    if (include_files)
    {
        node = current->files;
        while (node)
        {
            if (count == index)
                return node;
            node = node->next;
            count++;
        }
    }

    return NULL;
}

void navigate_directory(DirectoryNode *root)
{
    DirectoryNode *current = root;
    DirectoryNode *parent = NULL;
    UndoInfo undo_info = {NULL, 0, NULL};
    int total_nodes = get_total_nodes(current->subdirs) + (show_files ? get_total_nodes(current->files) : 0);
    int directories_per_page = terminal_height - 4;
    int total_pages = (total_nodes + directories_per_page - 1) / directories_per_page;
    int page = 1;
    int selected_index = 1;

    while (1)
    {
        display_directories(current, (page - 1) * directories_per_page + 1, selected_index, page, total_pages);

        int ch = getch();
        switch (ch)
        {
        case KEY_DOWN:
            if (selected_index < total_nodes)
            {
                selected_index++;
                if (selected_index > page * directories_per_page)
                {
                    page++;
                }
            }
            else
            {
                selected_index = 1;
                page = 1;
            }
            break;
        case KEY_UP:
            if (selected_index > 1)
            {
                selected_index--;
                if (selected_index <= (page - 1) * directories_per_page)
                {
                    page--;
                }
            }
            else
            {
                selected_index = total_nodes;
                page = total_pages;
            }
            break;
        case KEY_RIGHT:
            if (page < total_pages)
            {
                page++;
                selected_index = (page - 1) * directories_per_page + 1;
            }
            else
            {
                page = 1;
                selected_index = (page - 1) * directories_per_page + 1;
            }

            break;
        case KEY_LEFT:
            if (page > 1)
            {
                page--;
                selected_index = (page - 1) * directories_per_page + 1;
            }
            else
            {
                page = total_pages;
                selected_index = (page - 1) * directories_per_page + 1;
            }
            break;
        case '\n':
        {
            DirectoryNode *node = get_node_at_index(current, selected_index, show_files);
            if (node && node->subdirs != NULL)
            {
                parent = current;
                current = node;
                total_nodes = get_total_nodes(current->subdirs) + (show_files ? get_total_nodes(current->files) : 0);
                total_pages = (total_nodes + directories_per_page - 1) / directories_per_page;
                page = 1;
                selected_index = 1;
            }
            break;
        }
        case 'b':
            if (current->parent != NULL)
            {
                current = current->parent;
                total_nodes = get_total_nodes(current->subdirs) + (show_files ? get_total_nodes(current->files) : 0);
                total_pages = (total_nodes + directories_per_page - 1) / directories_per_page;
                page = 1;
                selected_index = 1;
            }
            break;
        case 't':
            show_files = !show_files;
            total_nodes = get_total_nodes(current->subdirs) + (show_files ? get_total_nodes(current->files) : 0);
            total_pages = (total_nodes + directories_per_page - 1) / directories_per_page;
            if (selected_index > total_nodes)
            {
                selected_index = total_nodes;
            }
            if (page > total_pages)
            {
                page = total_pages;
            }
            break;
        case 'x':
        {
            DirectoryNode *node = get_node_at_index(current, selected_index, show_files);

            if (node)
            {
                int confirm_height = 5;
                int confirm_width = 40;
                int confirm_y = (terminal_height - confirm_height) / 2;
                int confirm_x = (terminal_width - confirm_width) / 2;

                WINDOW *confirm_win = newwin(confirm_height, confirm_width, confirm_y, confirm_x);
                box(confirm_win, 0, 0);
                mvwprintw(confirm_win, 1, 2, "Are you sure you want to delete:");
                mvwprintw(confirm_win, 2, 2, "%s", strrchr(node->path, '/') ? strrchr(node->path, '/') + 1 : node->path);
                mvwprintw(confirm_win, 3, 2, "(y/N)");
                wrefresh(confirm_win);

                int confirm = wgetch(confirm_win);
                delwin(confirm_win);

                if (tolower(confirm) == 'y')
                {
                    undo_info.node = node;
                    undo_info.is_file = (node->subdirs == NULL);
                    undo_info.parent = current;

                    if (undo_info.is_file)
                    {
                        if (remove(node->path) == -1)
                        {
                            perror("remove");
                        }
                        else
                        {
                            remove_node_from_list(&current->files, node);
                            free(node);
                        }
                    }
                    else
                    {
                        delete_directory(node);
                        remove_node_from_list(&current->subdirs, node);
                    }
                }

                total_nodes = get_total_nodes(current->subdirs) + (show_files ? get_total_nodes(current->files) : 0);
                total_pages = (total_nodes + directories_per_page - 1) / directories_per_page;
                if (selected_index > total_nodes)
                {
                    selected_index = total_nodes;
                }
                if (selected_index < 1)
                {
                    selected_index = 1;
                }
                if (page > total_pages)
                {
                    page = total_pages;
                }
            }
            break;
        }
        /* case 'u':
            if (undo_info.node)
            {
                if (undo_info.is_file)
                {
                    undo_info.node->next = undo_info.parent->files;
                    undo_info.parent->files = undo_info.node;
                }
                else
                {
                    undo_info.node->next = undo_info.parent->subdirs;
                    undo_info.parent->subdirs = undo_info.node;
                }
                undo_info.node = NULL;
            }
            break; */
        case 'r':
        {
            DirectoryNode *node = get_node_at_index(current, selected_index, show_files);

            if (node)
            {
                reveal_in_finder(node->path);
            }
            break;
        }
        case 'h':
            show_help();
            break;
        case 'd':
            show_debug("test");
            break;
        case 'q':
            return;
        case '-':
            return;
        default:
            if (isdigit(ch))
            {
                char input[10] = {0};
                input[0] = ch;
                echo();
                getnstr(input + 1, sizeof(input) - 2);
                noecho();

                int choice;
                if (sscanf(input, "%d", &choice) == 1)
                {
                    if (choice == 0)
                    {
                        if (current->parent == NULL)
                        {
                            mvprintw(terminal_height - 1, 0, "Already at root directory. Press any key to continue.");
                            getch();
                        }
                        else
                        {
                            current = current->parent;
                            total_nodes = get_total_nodes(current->subdirs) + (show_files ? get_total_nodes(current->files) : 0);
                            total_pages = (total_nodes + directories_per_page - 1) / directories_per_page;
                            page = 1;
                            selected_index = 1;
                        }
                    }
                    else if (choice > 0 && choice <= total_nodes)
                    {
                        DirectoryNode *node = get_node_at_index(current, choice, show_files);

                        if (node)
                        {
                            parent = current;
                            current = node;
                            total_nodes = get_total_nodes(current->subdirs) + (show_files ? get_total_nodes(current->files) : 0);
                            total_pages = (total_nodes + directories_per_page - 1) / directories_per_page;
                            page = 1;
                            selected_index = 1;
                        }
                    }
                }
            }
            break;
        }
    }
}

void show_help()
{
    int help_height = 10;
    int help_width = 50;
    int help_y = (terminal_height - help_height) / 2;
    int help_x = (terminal_width - help_width) / 2;

    WINDOW *help_win = newwin(help_height, help_width, help_y, help_x);
    box(help_win, 0, 0);
    mvwprintw(help_win, 1, 2, "Help Menu:");
    mvwprintw(help_win, 2, 2, "'q' - Quit");
    mvwprintw(help_win, 3, 2, "'x' - Delete");
    mvwprintw(help_win, 4, 2, "'t' - Toggle Files/Folders");
    mvwprintw(help_win, 5, 2, "'b' - Go Back");
    mvwprintw(help_win, 6, 2, "'h' - Show Help");
    mvwprintw(help_win, 7, 2, "'u' - Undo Delete");
    mvwprintw(help_win, 8, 2, "'r' - Reveal in Finder");
    mvwprintw(help_win, 9, 2, "Use arrow keys to navigate and Enter to select");
    wrefresh(help_win);
    wgetch(help_win);
    delwin(help_win);
}

void remove_node_from_list(DirectoryNode **head, DirectoryNode *node_to_remove)
{
    if (*head == node_to_remove)
    {
        *head = node_to_remove->next;
    }
    else
    {
        DirectoryNode *prev = *head;
        while (prev && prev->next != node_to_remove)
        {
            prev = prev->next;
        }
        if (prev)
        {
            prev->next = node_to_remove->next;
        }
    }
}

void show_debug(const char *debugging)
{
    int help_height = 5;
    int help_width = 60;
    int help_y = (terminal_height - help_height) / 2;
    int help_x = (terminal_width - help_width) / 2;

    WINDOW *debugging_win = newwin(help_height, help_width, help_y, help_x);
    box(debugging_win, 0, 0);
    mvwprintw(debugging_win, 0, 2, "Debugging Menu:");
    mvwprintw(debugging_win, 1, 2, "debugging info");
    mvwprintw(debugging_win, 3, 2, "%s", debugging);
    wrefresh(debugging_win);
    wgetch(debugging_win);
    delwin(debugging_win);
}

DirectoryNode *copy_node(const DirectoryNode *node)
{
    if (!node)
        return NULL;
    DirectoryNode *new_node = (DirectoryNode *)malloc(sizeof(DirectoryNode));
    if (!new_node)
        return NULL;
    memcpy(new_node, node, sizeof(DirectoryNode));
    new_node->next = NULL;
    new_node->subdirs = NULL;
    new_node->files = NULL;
    return new_node;
}