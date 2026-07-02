#ifndef PLAYER_H
#define PLAYER_H
#include <stdint.h>
#include "dungeon.h"
//#include "main.h" //no longer needed for struct Player
struct Player {
    uint16_t x;
    uint16_t y;
};

enum Move_Direction {
    MV_N = 8, //"move north"
    MV_NE = 9,
    MV_E = 6,
    MV_SE = 3, //"move southeast" ...etc
    MV_S = 2,
    MV_SW = 1,
    MV_W = 4,
    MV_NW = 7,
    MV_INVALID = 99,
};

int player_move(struct Player *p, struct Dungeon_Context *c, enum Move_Direction d);
enum Move_Direction player_keypress_to_move(int keypress);

#endif //PLAYER_H
