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

//currently unused - TODO - set screen height and width manually
#define SCREEN_HEIGHT 50
#define SCREEN_WIDTH 80


int main()
{
	display_libraries_init();
    struct State gamestate;
	gamestate.dungeon_state = (struct Dungeon_Context){
		.width = 80,
		.height = 50,
	};
	enum Error_Type error_result = dungeon_generate(&gamestate.dungeon_state);
	if (error_result != E_OK) {
		display_libraries_end();
		fprintf(stderr, "Failed to generate dungeon properly.\n");
		return 1;
	}

	dungeon_display(&gamestate.dungeon_state);
	gamestate.player.y = (gamestate.dungeon_state.height >> 1);
	gamestate.player.x = (gamestate.dungeon_state.width >> 1);
	do {
		//code to run once per game cycle here
	} while ( game_loop(&gamestate) );

	display_libraries_end();
	return 0;
}

bool game_loop(struct State *s)
{
	bool should_continue = true;

    display_mvprintw(s->player.y, s->player.x, "@");
	display_refresh();

	const int keystroke = display_getch();	//pause before exit
    //if (input == 'q' || input == 'Q') should_continue = false;
    enum Input_Result key_result = input_handle_keystroke(keystroke);
    if (key_result == IR_QUIT) should_continue = false;
    else if (key_result == IR_MOVE) {
        enum Move_Direction md = player_keypress_to_move(keystroke);
        //fprintf(stderr, "md=%d\n", (int) md);
        if (md != MV_INVALID) player_move(&(s->player), &(s->dungeon_state), md);
    } else if (key_result == IR_NONE) { ; }
    else {
        assert(0); //ERROR! Unhandled Input_Result option.
    }

	return should_continue;
}
