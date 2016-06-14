//gcc canvas.c -o test -lncurses
#include <curses.h>
#include <string.h>
 
int main(void)
{
    static const char msg[] = "Press Q to Quit";
    int c;
 
    /* Start curses. Also sets LINES and COLS. */
    initscr();
 
    /* Pass every character immediately, not buffered. */
    cbreak();
 
    do {
        /* Clear the screen. */
        erase();
 
        /* Print "X" at the center of the screen, */
        mvaddch(LINES/2, COLS/2, 'X');
 
        /* and msg centered at the bottom. */
        mvaddstr(LINES-1, (COLS-strlen(msg))/2, msg);
 
        /* Move the cursor to the center of the screen too. */
        move(LINES/2, COLS/2);
 
        /* Refresh the screen. */
        refresh();
 
        /* Wait for a keypress. */
        c = getch();
 
    } while (c != 'Q' && c != 'q');
 
    /* Done. Return the terminal back to its normal state. */
    endwin();
 
    return 0;
}
