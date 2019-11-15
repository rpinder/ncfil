#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

void bomb(char *msg);
int get_files_in_directory(size_t N, size_t M, char files[N][M], char *directory);
void drawmenu(WINDOW *win, int item, size_t N, size_t M, char files[N][M], int counter, int rowoffset);
void loop(WINDOW *win, WINDOW* hwin, char file[], char dir[], int *rowoffset);
void drawHelp(WINDOW *win);

int main(void)
{
    WINDOW *mainwindow, *titlebar, *container, *helpwindow;

    newterm(NULL, stderr, stdin);
    refresh();
    noecho();
    curs_set(0);

    int maxx,maxy;
    getmaxyx(stdscr,maxy,maxx);

    if ( (container = newwin(maxy-1,maxx,1,0)) == NULL )
        bomb("unable to allocate memory for container window");
    if ( (titlebar = newwin(1, maxx,0,0)) == NULL )
        bomb("Unable to allocate memory for titlebar window");
    if ( (mainwindow = subwin(container,maxy-3,maxx-2,2,1)) == NULL )
        bomb("Unable to allocate memory for mainwindow");
    if ( (helpwindow = newwin(maxy-9,maxx-20,4,10)) == NULL )
        bomb("Unable to allocate memory for helpwindow");

    scrollok(mainwindow, 1);

    char *titlemsg = "ncfil - Ncurses File Browser";
    mvwaddstr(titlebar, 0, (maxx - strlen(titlemsg)) / 2, titlemsg);
    wrefresh(titlebar);

    box(container,0,0);
    wrefresh(container);

    int rowoffset;

    char file[95];
    char dir[1000] = "./";
    loop(mainwindow, helpwindow, file, dir, &rowoffset);

    while (1) {
        int i;
        for (i = 1; file[i] != '\0'; i++);
        if (file[i-1] == '/') {
            strcat(dir, file);
            loop(mainwindow, helpwindow, file, dir, &rowoffset);
        } else {
            break;
        }
    }

    endwin();
    printf("%s%s", dir, file);
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
        for (int i = 0;((dir = readdir(d)) != NULL) && i < N; i++) {
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

void drawmenu(WINDOW *win, int item, size_t N, size_t M, char files[N][M], int counter, int rowoffset)
{
    int maxx, maxy;
    getmaxyx(win,maxy,maxx);

    wclear(win);
    for (int i = 0; (i < counter && i < maxy); i++) {
        if (i == item)
            wattron(win, A_REVERSE);
        if (i + rowoffset < N)
            mvwaddstr(win,1+i,2,files[i + rowoffset]);
        wattroff(win, A_REVERSE);
    }
    wrefresh(win);
}

void loop(WINDOW *win, WINDOW *hwin, char file[], char dir[], int *rowoffset )
{
    const size_t N = 1000;
    const size_t M = 100;
    char files[N][M];

    *rowoffset = 0;

    int maxx, maxy;
    getmaxyx(win, maxy, maxx);

    int counter = get_files_in_directory(N, M, files, dir);
    int menuitem = 0;
    drawmenu(win, menuitem, N, M, files, counter, *rowoffset);
    wrefresh(win);

    int key;

    do {
        key = getch();
        switch(key) {
        case 'j':
            menuitem++;
            if (menuitem > maxy - 2 || menuitem > counter-1) {
                menuitem = maxy > (counter-1) ? (counter-1) : maxy-2;
                if (menuitem == maxy-2 && (menuitem+*rowoffset) < (counter - 1) )
                    (*rowoffset)++;
            }
            break;
        case 'k':
            menuitem--;
            if (menuitem < 0) {
                menuitem = 0;   
                if (*rowoffset > 0)
                    (*rowoffset)--;
            }
            break;
        case 't':
            menuitem = 0;
            *rowoffset = 0;
            break;
        case 'b':
            if (maxy < (counter - 1)) {
                menuitem = maxy-2;
                *rowoffset = counter - maxy + 1;
            } else {
                menuitem = counter - 1;
            }
            break;
        case 'h':
            drawHelp(hwin);
            break;
        default:
            break;
        }
        drawmenu(win, menuitem, N, M, files, counter, *rowoffset);
    } while (!(key == 'q' || key == '\n' || key == 'u'));
    if (key == 'q') {
        endwin();
        exit(0);
    } else if (key == 'u') {
        strcpy(file, "../");
    } else {
        strcpy(file, files[menuitem + *rowoffset]);
    }
}

void drawHelp(WINDOW *win)
{
    box(win,0,0);
    touchwin(win);
    mvwaddstr(win,0,3,"HELP-MENU");

    wrefresh(win);

    int key;
    do {
        key = getch();
        
    } while (!(key == 'q' || key == 'h'));
    if (key == 'q') {
        endwin();
        exit(0);
    }
}
