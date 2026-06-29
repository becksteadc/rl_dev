/* Colton Beckstead - 2026
 *
 * This file handles interfacing with the "platform-dependent" display code,
 * in this case, the curses library. (PDCurses specifically; its licensing is
 * more open, so I chose it.)
 *
 * The idea is that this file could be swapped out for another display.c / .o
 * file that provides the same functionality, but with a different display frontend
 * such as SDL, LIBTCOD, or a proper graphical engine.
 *
 * All functions in this file shall begin with display_ as a namespace indicator.
 *
 */


#include "display.h"
#include "../game_layer/error_defs.h"
#include "../translation_curses/platform_input.h"
#include <curses.h>
void exit(int); //No need to include stdlib just for this one function declaration.
int display_errno = 0;

/* All of the display code of the application is to be linked in from this file.
 * This should result in some amount of portability: simply replace the implementation
 * of these functions with another display library and keep the other existing code.
 * 
 * All functions shall begin with display_ as a namespace indicator.
 * */

void display_libraries_end(void)
{
	attroff(COLOR_PAIR(1));
	endwin();	//curses cleanup
}

void display_libraries_init(void)
{
	initscr();	//the master init for curses
	noecho(); 	//don't echo user input to screen
	cbreak();	//unbuffered input, C-c and C-z retained
	keypad(stdscr, true);	//enable numpad and fn keys
	curs_set(1);	//set cursor to normal
	if (has_colors() == false) {
		endwin();
		printf("%s\n", "Your terminal doesn't support colors");
		exit(1);
	}
	
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLUE); //background and foreground colors
	attron(COLOR_PAIR(1));
}

void display_getmaxyx(int *y, int *x)
{
    getmaxyx(stdscr, *y, *x);
}

int display_mvprintw(int y, int x, char *str)
{
    const int val = mvprintw(y, x, str);
    if (val == ERR) return E_DISPLAY;
    return 0; //success
}

void display_refresh(void)
{
    refresh();
}

int display_getch(void)
{
    return platform_input_map_keystroke(getch());
}
