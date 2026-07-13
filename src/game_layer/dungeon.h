#ifndef DUNGEON_H
#define DUNGEON_H

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "../platform_curses/display.h"
#include "error_defs.h"
#include "log.h"

#define DUNGEON_MAX_WIDTH 1024
#define DUNGEON_MAX_HEIGHT 512
#define PLACEMENT_MAX_ATTEMPTS 20
#define PLACEMENT_MAX_ROOMS 5

enum Dungeon_Type {
    DT_MORIA_TRADITIONAL, //Mostly square rooms (maybe eventually support limited special - tile vaults...)
	DT_UNUSED,
};

enum Dungeon_Flags {
	FL_NONE, //May not actually be necessary
	FL_NOMOVE,
	FL_LIT,
};
/* -- Note: commenting this preset section out, since it would cause redefinition linker errors
//Note: use Preset_Empty instead of hardcoded zero, since
//"empty" tiles require some fields to be set to be valid.
struct {
	Preset_Empty = 0x0000'2E'0000'0000'00; //separators correspond to Tile_Type struct fields
	Preset_Void = 0x0000'20'0000'0000'00; //Truly empty: character is a space
} Tile_Presets;
*/

union Tile_Type {
    uint64_t raw;
    struct {
        uint8_t flags; //bit map of tile flags; May end up being too small - Could be clever, and handle this like old SNES palettes: only have 8 possible flags, but have the meaning of those flags depend on a different context (set per-screen in SNES, / set per level here. -- Doesn't have to be per level, could also be per "chunk" of a level if needed...)
        uint16_t item; //A handle into a global item list
        uint16_t entity; //A handle into a global entity list -> 65536 possible concurrent entities should be more than enough. //? - is it good to have entity referenced from a tile? Or is keeping that in sync with global entity lists going to be a pain?
        uint8_t symbol; //ASCII symbol this tile is represented by
		uint8_t room_id; //Can tag a tile as part of a larger "room" or "collection" of tiles
						 //255 is reserved as a "non-value"
        uint8_t reserved;
    };
};
#define DUNGEON_TILE_NO_ROOM 255

struct Dungeon_Room {
	uint8_t id;
	uint16_t tile_ownership_len;
	uint32_t tile_ownership_offset;
};

//The max number of connections to other rooms a single room may have.
//16 is probably overkill ("node" because these are primarily for build graph logic)
#define DUNGEON_NODE_MAX_CONNS 4
#define DUNGEON_MAX_NODES 16
#define DUNGEON_NODE_NO_CONN 255

///Represents the context for a given dungeon level
//could make this more efficient with "programming without pointers" / 1st normal form
//technique, then could serialize the struct bytes directly to and from disk... TODO
//The direct size of this struct is not super important, as there will only be one in memory
struct Dungeon_Context {
    enum Dungeon_Type type;
    uint16_t width;
    uint16_t height;
    union Tile_Type *tile_array; //populated by dungeon generation functions
	struct Dungeon_Room rooms[DUNGEON_MAX_NODES];
}; //TODO - add more fields to the context as needed.

struct Dungeon_Build_Graph {
	uint8_t node_count;
	//Adjacency list for node connections: each entry goes like this:
	//[node_id][connection] = id of node being connected to
	//node ids are uint8_t handles into the build graph structure
	uint8_t node_connections[DUNGEON_MAX_NODES][DUNGEON_NODE_MAX_CONNS];
};

enum Error_Type dungeon_generate(struct Dungeon_Context *c);
enum Error_Type dungeon_gen_rooms(struct Dungeon_Context *c, enum Dungeon_Type dt);
//enum Error_Type dungeon_gen_blankslate(struct Dungeon_Context *c); //Depreciated by filledslate
void dungeon_generate_graph(struct Dungeon_Build_Graph *bg);
void dungeon_display(struct Dungeon_Context *c);
void dungeon_dealloc(struct Dungeon_Context *c);
uint32_t dungeon_yx_to_offset(struct Dungeon_Context *c, uint16_t y, uint16_t x);
void dungeon_debug_build_graph(struct Dungeon_Build_Graph *bg);
enum Error_Type dungeon_gen_filledslate(struct Dungeon_Context *c,
	union Tile_Type fill_tile);

enum Error_Type dungeon_place_moria_room (struct Dungeon_Context *c,
	uint8_t room_number, uint32_t corner_A_x, uint32_t corner_A_y,
	uint32_t corner_B_x, uint32_t corner_B_y);
enum Error_Type dungeon_place_moria_room_two(struct Dungeon_Context *c,
	uint8_t room_id, uint32_t x, uint32_t y);
enum Error_Type dungeon_dump_to_file(struct Dungeon_Context *c, char *filename);

void dungeon_place_corridor(struct Dungeon_Context *c, uint8_t corridor_id,
	uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2);
//TODO - this will be moved to another source file
int weighted_random(uint8_t *weights, uint8_t weight_count);

#endif //DUNGEON_H
