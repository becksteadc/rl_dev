/* Header for display.c
 * Note: header files should only include functions and extern declarations
 * as opposed to standard library code and other header imports.
 *
 */

#ifndef DISPLAY_H
#define DISPLAY_H

void display_libraries_init(void);
void display_libraries_end(void);
void display_getmaxyx(int *y, int *x);
int display_mvprintw(int y, int x, char *str);
void display_refresh(void);
int display_getch(void);

#endif //DISPLAY_H
