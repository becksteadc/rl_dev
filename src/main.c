#include "main.h"

int main() {
	initscr();	//the master init for curses
	noecho(); 	//don't echo user input to screen
	cbreak();	//unbuffered input, C-c and C-z retained
	keypad(stdscr, true);	//enable numpad and fn keys
	curs_set(1);	//set cursor to normal

	if (has_colors() == false) {
		endwin();
		printf("%s\n", "Your terminal doesn't support colors");
		return 1;
	}

	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLUE); //background and foreground colors
	attron(COLOR_PAIR(1));
	int x, y;
	getmaxyx(stdscr, y, x);
	y *= (int) 0.5;
	x = x * (int) 0.5 - 6;
	mvprintw(y, x, "Hello, world!");
	mvprintw(2, 2, "Hello, world!");
	refresh();

	attroff(COLOR_PAIR(1));

	getch();	//pause before exit
	endwin();	//ncurses cleanup
	return 0;
}
