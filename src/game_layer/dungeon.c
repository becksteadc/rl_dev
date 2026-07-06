#include "dungeon.h"

enum Error_Type dungeon_generate(struct Dungeon_Context *c)
{
	srand((unsigned int) time(NULL));
	union Tile_Type wall_tile = {
		.symbol = '#',
		.flags = FL_NOMOVE,
		.entity = 0,
		.item = 0,
	};
	enum Error_Type res = dungeon_gen_filledslate(c, wall_tile);
	if (res == E_OK)
		res = dungeon_gen_rooms(c, MORIA_TRADITIONAL);
	return res;
}

///MUST have srand already called.
enum Error_Type dungeon_gen_rooms(struct Dungeon_Context *c, enum Dungeon_Type dt)
{
	//struct Dungeon_Build_Graph = dungeon_gen_build_graph(c);
	switch (dt) {
	case MORIA_TRADITIONAL:
		uint16_t placement_failures = 0;
		uint8_t placed_rooms = 0;
		while (placement_failures < PLACEMENT_MAX_ATTEMPTS && placed_rooms < PLACEMENT_MAX_ROOMS) {
			uint32_t corner_A_x = (uint32_t) rand() % c->width;
			uint32_t corner_A_y = (uint32_t) rand() % c->height;
			uint32_t corner_B_x = (uint32_t) rand() % c->width;
			uint32_t corner_B_y = (uint32_t) rand() % c->height;
			if (
				//Ugly hack using placed_rooms as the room ID here... XXX
				dungeon_place_moria_room(c, placed_rooms, corner_A_x, corner_A_y, corner_B_x, corner_B_y)
				== E_OK
			) { ++placed_rooms; }
			else ++placement_failures;
		}
	break;
	default:
		return E_UNIMPLEMENTED;
	}
	return E_OK;
}

enum Error_Type dungeon_dump_to_file(struct Dungeon_Context *c, char *filename)
{
	FILE *fp = fopen(filename, "w");
	if (fp == NULL) return E_OS;

	uint32_t tile_array_len = c->width * c->height;
	for (uint32_t i = 0; i < tile_array_len; ++i) {
		if (i % c->width == 0) fputc('\n', fp);
		fputc((c->tile_array + i)->symbol, fp);
	}
	if (fclose(fp) == 0)
		return E_OK;
	else
		return E_OS;
}


//Might not be the best to go about placing individual rooms like this.
//How about dungeon_generate_graph() instead, which creates an abstract
//connected graph of rooms that can be iterated upon until no more can be added?

///Places the completed build graph into the parameter bg
///Very coupled with the specific implementation of the Dungeon_Build_Graph struct
///So don't assume any sort of separation and abstraction there.
void dungeon_generate_graph(struct Dungeon_Build_Graph *bg)
{
	bg->node_count = 0;
	memset(bg->node_connections, DUNGEON_NODE_NO_CONN, sizeof(bg->node_connections));
	uint8_t current_node = 0;
	while (bg->node_count <= DUNGEON_MAX_NODES) {
		int i = 0;

		//scrub to the end of the current node's connections
		for (; bg->node_connections[current_node][i] != DUNGEON_NODE_NO_CONN; ++i) { ; }
		if (i >= DUNGEON_NODE_MAX_CONNS) {
			uint8_t conn_to_follow = (uint8_t) (rand() % i);
			current_node = bg->node_connections[current_node][conn_to_follow];
			continue; //Have to move on to a different node
		}
		++(bg->node_count);
		bg->node_connections[current_node][i] = bg->node_count;
	}
	return;
}

///If negative values are passed in, the behavior is undefined.
///srand *must* have been called already
int weighted_random(uint8_t *weights, uint8_t weight_count)
{
	int param_total_sum = 0;
	for (int i = 0; i < weight_count; ++i) {
		param_total_sum += *(weights+i);
	}
	int randval = rand();
	//is the +1 needed on the line below? XXX
	randval = (randval % param_total_sum) + 1;
	//fprintf(stdout, "randval=%d, param_total_sum = %d\n", randval, param_total_sum);
	param_total_sum = 0;
	for (int i = 0; i < weight_count; ++i) {
		param_total_sum += *(weights + i);
		if (param_total_sum >= randval) {
			return i;
		}
	}
	fprintf(stderr, "Hmm, might be a bug in weighted random\n");
	return 0;
}

void dungeon_debug_build_graph(struct Dungeon_Build_Graph *bg)
{
	for (int i = 0; i < DUNGEON_MAX_NODES; ++i) {
		int j = 0;
		fprintf(stdout, "Node %d\n", i);
		while (bg->node_connections[i][j] != DUNGEON_NODE_NO_CONN) {
			if (j > DUNGEON_NODE_MAX_CONNS) {
				fprintf(stderr, "Overran node max connection limit. Exiting");
				return;
			}
			fprintf(stdout, "\tConnection: %d\n", bg->node_connections[i][j]);
			++j;
		}
	}
}

//srand must have been previously called. Implicitly coupled to dungeon_generate, which does so
enum Error_Type dungeon_place_moria_room (
	struct Dungeon_Context *c,
	uint8_t room_number,
	uint32_t corner_A_x,
	uint32_t corner_A_y,
	uint32_t corner_B_x,
	uint32_t corner_B_y
) {
	uint32_t start_x, start_y;
	uint32_t end_x, end_y;
	if (corner_A_x > corner_B_x) {
		start_x = corner_B_x;
		end_x = corner_A_x;
	} else {
		start_x = corner_A_x;
		end_x = corner_B_x;
	}
	if (corner_A_y > corner_B_y) {
		start_y = corner_B_y;
		end_y = corner_A_y;
	} else {
		start_y = corner_A_y;
		end_y = corner_B_y;
	}


	union Tile_Type blank_floor = {
		.symbol = '.',
		.flags = 0,
		.item = 0,
		.entity = 0,
		.room_id = room_number,
	};
	for (uint32_t i = start_y; i < end_y; ++i) {
		for (uint32_t j = start_x; j < end_x; ++j) {
			*(c->tile_array + i * c->width + j) = blank_floor;
		}
	}
	/*
	for (uint32_t i = start_x; i <= end_x; ++i) {
		*(c->tile_array + start_y * c->width + i) = blank_floor;
		*(c->tile_array + end_y * c->width + i) = blank_floor;
	}
	for (uint32_t i = start_y; i <= end_y; ++i) {
		*(c->tile_array + i * c->width + start_x) = blank_floor;
		*(c->tile_array + i * c->width + end_x) = blank_floor;
	}*/
	
	end_y++;
	start_y = end_y;

	return E_OK;
}

enum Error_Type dungeon_gen_filledslate(struct Dungeon_Context *c, union Tile_Type fill_tile)
{
	if (c->width > DUNGEON_MAX_WIDTH) c->width = DUNGEON_MAX_WIDTH;
	if (c->height > DUNGEON_MAX_HEIGHT) c->height = DUNGEON_MAX_HEIGHT;
	c->tile_array = (union Tile_Type *) malloc(
		sizeof(struct Dungeon_Context) * c->width * c->height
	);
	if (c->tile_array == NULL) {
		return E_OS;
	}
	for (int i = 0; i < c->height * c->width; ++i) {
		*(c->tile_array + i) = fill_tile;
	}

	return E_OK;
}

/* //Depreciated by dungeon_gen_filledslate -> just pass in a "blank tile" option
enum Error_Type dungeon_gen_blankslate(struct Dungeon_Context *c)
{
	if (c->width > DUNGEON_MAX_WIDTH) c->width = DUNGEON_MAX_WIDTH;
	if (c->height > DUNGEON_MAX_HEIGHT) c->height = DUNGEON_MAX_HEIGHT;
	c->tile_array = (union Tile_Type *) malloc(
		sizeof(struct Dungeon_Context) * c->width * c->height
	);
	if (c->tile_array == NULL) {
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
		//if (i % 6 == 0) {
		if (false) { //NOTE: debug / testing code
			*(c->tile_array + i) = (union Tile_Type){
				.flags = FL_NOMOVE,
				.item = 0,
				.entity = 0,
				.symbol = '#',
			};
		} else {
			*(c->tile_array + i) = blank_floor;
		}
	}

	//Just generated an empty grid for now. Returning...
	return E_OK;
}
}
*/
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
