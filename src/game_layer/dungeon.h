#ifndef DUNGEON_H
#define DUNGEON_H

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "../platform_curses/display.h"
#include "error_defs.h"

#define DUNGEON_MAX_WIDTH 1024
#define DUNGEON_MAX_HEIGHT 512

enum Dungeon_Type {
    MORIA_TRADITIONAL, //Mostly square rooms (maybe eventually support limited special - tile vaults...)
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
        uint8_t flags; //may end up being too small - Could be clever, and handle this like old SNES palettes: only have 8 possible flags, but have the meaning of those flags depend on a different context (set per-screen in SNES, / set per level here. -- Doesn't have to be per level, could also be per "chunk" of a level if needed...)
        uint16_t item; //A handle into a global item list
        uint16_t entity; //A handle into a global entity list -> 65536 possible concurrent entities should be more than enough. //? - is it good to have entity referenced from a tile? Or is keeping that in sync with global entity lists going to be a pain?
        uint8_t symbol; //ASCII symbol this tile is represented by
        uint16_t reserved;
    };
};

///Represents the context for a given dungeon level
struct Dungeon_Context {
    enum Dungeon_Type type;
    uint16_t width;
    uint16_t height;
    union Tile_Type *tile_array; //populated by dungeon generation functions
}; //TODO - add more fields to the context as needed.


enum Error_Type dungeon_generate(struct Dungeon_Context *c);
void dungeon_display(struct Dungeon_Context *c);
void dungeon_dealloc(struct Dungeon_Context *c);
uint32_t dungeon_yx_to_offset(struct Dungeon_Context *c, uint16_t y, uint16_t x);

#endif //DUNGEON_H
