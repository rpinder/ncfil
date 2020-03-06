#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

void bomb(char *msg);
int get_files_in_directory(size_t N, size_t M, char files[N][M], char *directory);
void sortFiles(size_t N, size_t M, char files[N][M], int counter);
void drawmenu(int item, size_t N, size_t M, char files[N][M], int counter, int rowoffset);
void loop(char file[], char dir[], int *rowoffset);
void help_window(int item, size_t N, size_t M, char files[N][M], int counter, int rowoffset);
void createWindows();
void start_ncurses();
void stop_ncurses();
void resize();
void drawhelp(int offset, char *helpfile, int line_count);
void lineWrap(char *file, char **newfile, int width, int *line_count);

WINDOW *mainwindow, *container, *helpwindow;

char *help = "A basic file browser using Ncurses\n.\nRunning 'ncfil' will output the relative filepath to the chosen file through stdout. The idea is for it to be used in conjunction with another command.\n.\nvim $(ncfil) - This will open ncfil and then open the chosen file in vim.\ncat $(ncfil) - This will open ncfil and then print the chosen file to stdout.\n.\nKeybindings\nJ - Move down\nK - Move up\nU - Move up a directory\nT - Move to top of list\nB - Move to bottom of list\nQ - Quit\nENTER - Select file/Go into directory\n";
    
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
    wrefresh(mainwindow);
    wrefresh(helpwindow);
}

void createWindows() {
    int maxx, maxy;
    getmaxyx(stdscr, maxy, maxx);

    if ((container = newwin(maxy, maxx, 0, 0)) == NULL)
        bomb("unable to allocate memory for container window");
    if ((mainwindow = subwin(container, maxy - 3, maxx - 2, 1, 1)) == NULL)
        bomb("Unable to allocate memory for mainwindow");
    if ((helpwindow = newwin(maxy - 9, maxx - 20, 4, 10)) == NULL)
        bomb("Unable to allocate memory for helpwindow");

    scrollok(mainwindow, 1);

    box(container, 0, 0);
    char *titlemsg = "ncfil-Ncurses-File-Browser";
    mvwaddstr(container, 0, (maxx - strlen(titlemsg)) / 2, titlemsg);
    wrefresh(container);
}

void start_ncurses()
{
    newterm(NULL, stderr, stdin);
    refresh();
    noecho();
    curs_set(0);
}

void stop_ncurses()
{
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
            help_window(menuitem, N, M, files, counter, *rowoffset);
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

void help_window(int item, size_t N, size_t M, char files[N][M], int counter, int rowoffset)
{
    int offset = 0;
    int maxy = getmaxy(helpwindow);
    int maxx = getmaxx(helpwindow);
    int line_count;
    char *helpfile;
    lineWrap(help, &helpfile, maxx-2, &line_count);
    drawhelp(offset, helpfile, line_count);
    int key;
    do {
        key = getch();
        switch (key) {
         case 'j':
             if (offset + maxy - 3 < line_count)
                 offset++;
            break;
        case 'k':
            offset--;
            if (offset < 0) {
                offset = 0;
            }
            break;
        case KEY_RESIZE:
            resize();
            int maxx = getmaxx(helpwindow);
            lineWrap(help, &helpfile, maxx-2, &line_count);
            drawmenu(item, N, M, files, counter, rowoffset);
            box(helpwindow,0,0);
            touchwin(helpwindow);
            mvwaddstr(helpwindow,0,3,"HELP-MENU");
            break;
        default:
            break;
        }
        drawhelp(offset, helpfile, line_count);
    } while (!(key == 'q' || key == 'h'));
    if (key == 'q') {
        endwin();
        exit(0);
    }
}

void drawhelp(int offset, char *helpfile, int line_count)
{
    wclear(helpwindow);
    touchwin(helpwindow);
    box(helpwindow,0,0);
    mvwaddstr(helpwindow,0,3,"HELP-MENU");
    wrefresh(helpwindow);
    int maxx = getmaxx(helpwindow);
    int maxy = getmaxy(helpwindow); 
    lineWrap(help, &helpfile, maxx-4, &line_count);

    char *token;
    token = strtok(helpfile, "\n");
    for (int i = 0; i < offset; i++) {
        token = strtok(NULL, "\n");
    }
    for (int i = 1; i < maxy - 2 && token != NULL; i++) {
        mvwaddstr(helpwindow, i, 1, token);
        token = strtok(NULL, "\n");
    }

    wrefresh(helpwindow);
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

void lineWrap(char *file, char **newfile, int width, int *line_count)
{
    *line_count = 0;
    *newfile = malloc(strlen(file)*sizeof(char));
    int col = 0;
    for (int i = 0; file[i] != '\0'; i++, col++) {
        (*newfile)[i] = file[i];
        if (file[i] == '\n') {
            col = 0;
            (*line_count)++;
        }
        if (col > width) {
            int j;
            for (j = i; file[j] != ' '; j--);
            (*newfile)[j] = '\n';
            (*line_count)++;
            col = 0;
        }
    }
    (*newfile)[strlen(file)] = '\0';
}
