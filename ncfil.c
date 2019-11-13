#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

void bomb(char *msg);
int get_files_in_directory(size_t N, size_t M, char files[N][M], char *directory);
void drawmenu(WINDOW *win, int item, size_t N, size_t M, char files[N][M], int counter);

int main(void)
{
    WINDOW *mainwindow, *titlebar, *container;

    initscr();
    refresh();
    noecho();
    curs_set(0);
    start_color();

    init_pair(1, COLOR_RED, COLOR_BLACK);

    int maxx,maxy;
    getmaxyx(stdscr,maxy,maxx);

    if ( (container = newwin(maxy-1,maxx,1,0)) == NULL )
        bomb("unable to allocate memory for container window");
    if ( (titlebar = newwin(1, maxx,0,0)) == NULL )
        bomb("Unable to allocate memory for titlebar window");
    if ( (mainwindow = subwin(container,maxy-3,maxx-2,2,1)) == NULL )
        bomb("Unable to allocate memory for subwindow");

    char *titlemsg = "ncfil - Ncurses File Manager - version 0.1";
    mvwaddstr(titlebar, 0, (maxx - strlen(titlemsg)) / 2, titlemsg);
    wrefresh(titlebar);

    box(container,0,0);
    wrefresh(container);

    const size_t N = 100;
    const size_t M = 100;
    char files[N][M];

    int counter = get_files_in_directory(N, M, files, ".");
    int menuitem = 0;
    drawmenu(mainwindow, menuitem, N, M, files, counter);
    wrefresh(mainwindow);

    int key;

    do {
        key = getch();
        switch(key) {
        case 'j':
            menuitem++;
            if (menuitem > counter-1) menuitem = 0;
            break;
        case 'k':
            menuitem--;
            if (menuitem < 0) menuitem = counter-1;
            break;
        default:
            break;
        }
        drawmenu(mainwindow, menuitem, N, M, files, counter);
    } while (key != 'q');

    endwin();
    return 0;
}

void bomb(char *msg)
{
    endwin();
    puts(msg);
    exit(1);
}

int get_files_in_directory(size_t N, size_t M, char files[N][M], char *directory)
{
    int counter = 0;
    DIR *d;
    char name[95];
    struct dirent *dir;
    d = opendir(directory);
    if (d) {
        for (int i = 0;(dir = readdir(d)) != NULL; i++) {
            strncpy(name, dir->d_name, sizeof(name));
            if (dir->d_type == 4)
                strcat(name, "/"); 
            strncpy(files[i], name, sizeof(files[0]));
            counter++;
        }
        closedir(d);
    }
    return counter;
}

void drawmenu(WINDOW *win, int item, size_t N, size_t M, char files[N][M], int counter)
{
    wclear(win);
    for (int i = 0; i < counter; i++) {
        if (i == item)
            wattron(win, A_REVERSE);
        mvwaddstr(win,1+i,2,files[i]);
        wattroff(win, A_REVERSE);
    }
    wrefresh(win);
}
