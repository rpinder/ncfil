#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

void bomb(char *msg);
int get_files_in_directory(size_t N, size_t M, char files[N][M], char *directory);
void sortFiles(size_t N, size_t M, char files[N][M], int counter);
void drawmenu(int item, size_t N, size_t M, char files[N][M], int counter, int rowoffset);
void loop(char file[], char dir[], int *rowoffset);
void drawHelp(int item, size_t N, size_t M, char files[N][M], int counter, int rowoffset);
void createWindows();
void start_ncurses();
void stop_ncurses();
void resize();


WINDOW *mainwindow, *titlebar, *container, *helpwindow;

int main(void)
{
    start_ncurses();

    createWindows();

    int rowoffset;

    char file[95];
    char dir[1000] = "./";
    loop(file, dir, &rowoffset);

    while (1) {
        int i;
        for (i = 1; file[i] != '\0'; i++);
        if (file[i-1] == '/') {
            strcat(dir, file);
            loop(file, dir, &rowoffset);
        } else {
            break;
        }
    }

    stop_ncurses();
    printf("%s%s", dir, file);
    return 0;
}

void resize() {
    stop_ncurses();
    start_ncurses();
    refresh();
    createWindows();
    wrefresh(container);
    wrefresh(titlebar);
    wrefresh(mainwindow);
    wrefresh(helpwindow);
}

void createWindows() {
    int maxx, maxy;
    getmaxyx(stdscr, maxy, maxx);

    if ((container = newwin(maxy - 1, maxx, 1, 0)) == NULL)
        bomb("unable to allocate memory for container window");
    if ((titlebar = newwin(1, maxx, 0, 0)) == NULL)
        bomb("Unable to allocate memory for titlebar window");
    if ((mainwindow = subwin(container, maxy - 3, maxx - 2, 2, 1)) == NULL)
        bomb("Unable to allocate memory for mainwindow");
    if ((helpwindow = newwin(maxy - 9, maxx - 20, 4, 10)) == NULL)
        bomb("Unable to allocate memory for helpwindow");

    scrollok(mainwindow, 1);

    char *titlemsg = "ncfil - Ncurses File Browser";
    mvwaddstr(titlebar, 0, (maxx - strlen(titlemsg)) / 2, titlemsg);
    wrefresh(titlebar);

    box(container, 0, 0);
    wrefresh(container);
}

void start_ncurses() {
    newterm(NULL, stderr, stdin);
    refresh();
    noecho();
    curs_set(0);
}

void stop_ncurses() {
    endwin();
}

void end_ncurses() {
    endwin();
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

void drawmenu(int item, size_t N, size_t M, char files[N][M], int counter, int rowoffset)
{
    int maxy = getmaxy(mainwindow);
    wclear(mainwindow);
    for (int i = 0; (i < counter && i < maxy); i++) {
        if (i == item)
            wattron(mainwindow, A_REVERSE);
        if (i + rowoffset < N)
            mvwaddstr(mainwindow,1+i,2,files[i + rowoffset]);
        wattroff(mainwindow, A_REVERSE);
    }
    wrefresh(mainwindow);
}

void loop(char file[], char dir[], int *rowoffset )
{
    const size_t N = 1000;
    const size_t M = 100;
    char files[N][M];

    *rowoffset = 0;

    int maxy = getmaxy(mainwindow);

    int counter = get_files_in_directory(N, M, files, dir);
    sortFiles(N, M, files, counter);
    int menuitem = 0;
    drawmenu(menuitem, N, M, files, counter, *rowoffset);
    wrefresh(mainwindow);

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
            drawHelp(menuitem, N, M, files, counter, *rowoffset);
            break;
        case KEY_RESIZE:
            resize();
            break;
        default:
            break;
        }
        drawmenu(menuitem, N, M, files, counter, *rowoffset);
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

void drawHelp(int item, size_t N, size_t M, char files[N][M], int counter, int rowoffset)
{
    box(helpwindow,0,0);
    touchwin(helpwindow);
    mvwaddstr(helpwindow,0,3,"HELP-MENU");

    wrefresh(helpwindow);

    int key;
    do {
        key = getch();
        if (key == KEY_RESIZE) {
            resize();
            drawmenu(item, N, M, files, counter, rowoffset);
            box(helpwindow,0,0);
            touchwin(helpwindow);
            mvwaddstr(helpwindow,0,3,"HELP-MENU");
            wrefresh(helpwindow);
        }
    } while (!(key == 'q' || key == 'h'));
    if (key == 'q') {
        endwin();
        exit(0);
    }
}

void sortFiles(size_t N, size_t M, char files[N][M], int counter)
{
    int holePosition;
    char valueToInsert[M];

    for (int i = 1; i < counter; i++) {
        strcpy(valueToInsert, files[i]);
        holePosition = i;

        while (holePosition > 0 && strcmp(files[holePosition - 1], valueToInsert) > 0) {
            strcpy(files[holePosition], files[holePosition - 1]);
            holePosition--;
        }

        if (holePosition != i) {
            strcpy(files[holePosition], valueToInsert);
        }
    }   
}
