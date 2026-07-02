#ifndef MAIN_HDR_GUARD
#define MAIN_HDR_GUARD

#include "dungeon.h"
#include "player.h"
struct State {
    struct Player player;
	struct Dungeon_Context dungeon_state;
};

bool game_loop(struct State *s);
#endif //MAIN_HDR_GUARD
