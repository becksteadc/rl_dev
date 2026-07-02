#include "dungeon.h"

enum Error_Type dungeon_generate(struct Dungeon_Context *c)
{
	if (c->width > DUNGEON_MAX_WIDTH) c->width = DUNGEON_MAX_WIDTH;
	if (c->height > DUNGEON_MAX_HEIGHT) c->height = DUNGEON_MAX_HEIGHT;
	c->tile_array = (union Tile_Type *) malloc(
		sizeof(struct Dungeon_Context) * c->width * c->height
	);
	if (c->tile_array == NULL) {
		//TODO - error logging - couldn't allocate memory for dungeon.
		return E_OS;
	}
	//To translate y,x coordinates to tile_array offset: y*c->width + x;
	union Tile_Type blank_floor = {
		.flags = 0,
		.item = 0,
		.entity = 0,
		.symbol = '.',
	};
	//memset(c->tile_array, blank_floor.raw, sizeof(struct Dungeon_Context) * c->width * c->height);
	for (int i = 0; i < c->height * c->width; ++i) {
		*(c->tile_array + i) = blank_floor;
	}

	//Just generated an empty grid for now. Returning...
	return E_OK;
}

void dungeon_display(struct Dungeon_Context *c)
{
	//const uint32_t dungeon_size = c->height * c->width;
	for (int y = 0; y < c->height; ++y) {
		for (int x = 0; x < c->width; ++x) {
			display_mvinsch(y, x, (*(c->tile_array + y * c->width + x)).symbol );
		}
	}
}

void dungeon_dealloc(struct Dungeon_Context *c)
{
	free(c->tile_array);
}

uint32_t dungeon_yx_to_offset(struct Dungeon_Context *c, uint16_t y, uint16_t x)
{
	uint32_t res = y;
	res *= c->width;
	res += x;
	return res;
	//return y * c->width + x;
}
