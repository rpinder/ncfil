#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

void bomb(char *msg);

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
