CC=gcc
OBJS=main.o display.o input.o platform_input.o
SRC=main.c
SRC_DIR=src
OBJ_DIR=objs
COMPILE_OPTS=-Wall -Wextra -Werror -Wshadow -Wconversion -Wunreachable-code
LINKER_OPTS=-lcurses

# Use this instead of CC directly
BUILD=$(CC) $(LINKER_OPTS) $(COMPILE_OPTS)

all:
	$(info Specify a backend to build for. Currently supported:)
	$(info curses (run "make curses"))

clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f game build/game
	rm -f $(OBJ_DIR)/curses/*.o

curses: $(OBJS) $(OBJ_DIR)/curses game
	$(BUILD) -o game $(OBJ_DIR)/curses/*.o $(OBJ_DIR)/*.o

$(OBJ_DIR)/curses:
	mkdir $(OBJ_DIR)/curses

main.o: $(SRC_DIR)/game_layer/main.c
	# "Game layer" object: output directly into $(OBJ_DIR)
	$(BUILD) -o $(OBJ_DIR)/main.o -c $(SRC_DIR)/game_layer/main.c

display.o: $(SRC_DIR)/platform_curses/display.c
	$(BUILD) -c -o $(OBJ_DIR)/display.o $(SRC_DIR)/platform_curses/display.c

platform_input.o: $(SRC_DIR)/translation_curses/platform_input.c $(SRC_DIR)/translation_curses/platform_input.h
	$(BUILD) -c -o $(OBJ_DIR)/curses/platform_input.o $(SRC_DIR)/translation_curses/platform_input.c

input.o: $(SRC_DIR)/game_layer/input.c $(SRC_DIR)/game_layer/input.h
	$(BUILD) -c -o $(OBJ_DIR)/input.o $(SRC_DIR)/game_layer/input.c
