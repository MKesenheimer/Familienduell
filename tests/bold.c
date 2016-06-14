#include <ncurses.h>

int main()
{
    initscr();

    attron(A_BOLD);
    printw("Hello.\n");

    attroff(A_BOLD);
    printw("World.\n");

    refresh();
    getch();
    endwin();
    return 0;
}
