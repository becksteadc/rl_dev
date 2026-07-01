/* Header for display.c
 * 
 * Note that this header file should contain only function signatures
 * that are platform-agnostic, as it is a common header that may be
 * used for multiple different "backends" / platforms.
 * 
 *
 */

#ifndef DISPLAY_H
#define DISPLAY_H
#include "../game_layer/error_defs.h"

void display_libraries_init(void);
void display_libraries_end(void);
void display_getmaxyx(int *y, int *x);
int display_mvprintw(int y, int x, char *str);
void display_refresh(void);
int display_getch(void);
enum Error_Type display_resize_window(int y, int x);

#endif //DISPLAY_H
