/* Colton Beckstead - 2026
 * Entry point for the program.
 * This file should not contain significant logic: game loop functions
 * should be outsourced to game.c (TODO) or other locations.
 *
 * Handling of command line arguments, initialization, and shutdown of
 * resources can all happen here.
 */


/* C standard library headers */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
//End C standard library headers

#include "main.h"
#include "platform_includes.h"
//#include "display.h" //display_ functions
#include "input.h"
#include "player.h" //for enum Move_Direction

//currently unused - TODO - set screen height and width manually
#define SCREEN_HEIGHT 50
#define SCREEN_WIDTH 80


bool game_loop(struct State *);

int main()
{
	display_libraries_init();
    struct State gamestate;
    display_getmaxyx(&gamestate.p.y, &gamestate.p.x);
    //getmaxyx(stdscr, gamestate.p.y, gamestate.p.x);
    gamestate.p.y >>= 1;
    gamestate.p.x >>= 1;
	do {
		//code to run once per game cycle here
	} while ( game_loop(&gamestate) );

	display_libraries_end();
	return 0;
}
bool game_loop(struct State *s)
{
	bool should_continue = true;

    display_mvprintw(s->p.y, s->p.x, "@");
	display_refresh();

	const int keystroke = display_getch();	//pause before exit
    //if (input == 'q' || input == 'Q') should_continue = false;
    enum Input_Result key_result = input_handle_keystroke(keystroke);
    if (key_result == IR_QUIT) should_continue = false;
    else if (key_result == IR_MOVE) {
        enum Move_Direction md = player_keypress_to_move(keystroke);
        //fprintf(stderr, "md=%d\n", (int) md);
        if (md != MV_INVALID) player_move(&(s->p), md);
    } else if (key_result == IR_NONE) { ; }
    else {
        assert(0); //ERROR! Unhandled Input_Result option.
    }

	return should_continue;
}
