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


int main(void)
{
//	struct Dungeon_Context throwaway = { .width = 80, .height = 50, };
//	enum Error_Type test_result = dungeon_generate(&throwaway);
//	if (error_result != E_OK) { fprintf(stderr, "throwaway err\n"); return 1; }

	struct Dungeon_Build_Graph bg;
	dungeon_generate_graph(&bg);
	dungeon_debug_build_graph(&bg);

	//	Weighted random testing nonsense. Can be removed later.
//	srand((unsigned int) time(NULL));
//	uint8_t wrand_test[4] = {5, 10, 5, 2};
//	for (int i = 0; i < 20; ++i) {
//		printf("wrand=%d\n", weighted_random(wrand_test, 4));
//	}
	getchar();

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

	if (dungeon_dump_to_file(&(gamestate.dungeon_state), "dungeon_dump.txt.ignore") != E_OK) {
		display_mvprintw(0, 0, "Failed to dump dungeon to file\n");
		display_getch();
	}
	dungeon_dealloc(&(gamestate.dungeon_state));
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
