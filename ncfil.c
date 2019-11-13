#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

void bomb(char *msg);
int get_files_in_directory(size_t N, size_t M, char files[N][M], char *directory);

int main(void)
{
    WINDOW *mainwindow, *titlebar;

    initscr();
    refresh();

    int maxx,maxy;
    getmaxyx(stdscr,maxy,maxx);

    if ( (mainwindow = newwin(maxy-1,maxx,1,0)) == NULL )
        bomb("unable to allocate memory for mainwindow window");
    if ( (titlebar = newwin(1, maxx,0,0)) == NULL )
        bomb("Unable to allocate memory for titlebar window");

    char *titlemsg = "ncfil - Ncurses File Manager - version 0.1";
    mvwaddstr(titlebar, 0, (maxx - strlen(titlemsg)) / 2, titlemsg);
    wrefresh(titlebar);

    box(mainwindow,0,0);

    const size_t N = 100;
    const size_t M = 100;
    char files[N][M];
    int counter = get_files_in_directory(N, M, files, ".");
    for (int i = 0; i < counter; i++) {
        mvwaddstr(mainwindow,2+i,2,files[i]);
    }
    
    wrefresh(mainwindow);

    getch();

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
    struct dirent *dir;
    d = opendir(directory);
    if (d) {
        for (int i = 0;(dir = readdir(d)) != NULL; i++) {
            strncpy(files[i], dir->d_name, sizeof(files[0]));
            counter++;
        }
        closedir(d);
    }
    return counter;
}
