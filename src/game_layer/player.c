#include <assert.h>
#include "player.h"
#include "input.h"
#include "platform_includes.h"
#include "dungeon.h"

//Updates the player coordinates based on a move direction.
//For now (until a display handling refactor is done) writes a
//blank character to where the player was previously standing.
//
//When actual map times are implemented, will have to write the correct
//character from those and such.
//
//The int return value is for later: when terrain is added, the player may
//have to "fail" to move. This can be indicated by a nonzero return value.
int player_move(struct Player *p, struct Dungeon_Context *c, enum Move_Direction d)
{
    const uint16_t old_x = p->x;
    const uint16_t old_y = p->y;
    switch(d) {
    case MV_N:
        p->y -= 1;
        break;
    case MV_NE:
        p->y -= 1;
        p->x += 1;
        break;
    case MV_E:
        p->x += 1;
        break;
    case MV_SE:
        p->y += 1;
        p->x += 1;
        break;
    case MV_S:
        p->y += 1;
        break;
    case MV_SW:
        p->y += 1;
        p->x -= 1;
        break;
    case MV_W:
        p->x -= 1;
        break;
    case MV_NW:
        p->y -= 1;
        p->x -= 1;
        break;
    default:
        assert(0); //Should be unreachable - an improper enum input.
    }
	if ((dungeon_yx_to_offset(c, p->y, p->x) + c->tile_array)->flags & FL_NOMOVE) {
		p->y = old_y;
		p->x = old_x;
		//TODO - print out a message that there is a wall in the way
		//TODO - set "free turn" flag, so that this keypress doesn't take player turn
	} else {
		//uint8_t hack[2] = {(dungeon_yx_to_offset(c, old_y, old_x) + c->tile_array)->symbol, '\0'};
		//display_mvprintw(old_y, old_x, (char *) hack);
		display_mvinsch(old_y, old_x, (dungeon_yx_to_offset(c, old_y, old_x) + c->tile_array)->symbol);
		display_mvdelch(old_y, old_x + 1); //HACK
		//let the caller refresh rather than doing so here - they may have more cached
		//updates to do
	}
    return 0;
}

enum Move_Direction player_keypress_to_move(int keypress)
{
    switch (keypress) {
    case gKEY_UP:
        return MV_N;
    case gKEY_NUMPAD_9:
        return MV_NE;
    case gKEY_RIGHT:
        return MV_E;
    case gKEY_NUMPAD_3:
        return MV_SE;
    case gKEY_DOWN:
        return MV_S;
    case gKEY_NUMPAD_1:
        return MV_SW;
    case gKEY_LEFT:
        return MV_W;
    case gKEY_NUMPAD_7:
        return MV_NW;
    default:
        return MV_INVALID;
    }
}
