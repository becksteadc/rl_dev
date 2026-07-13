# WARNING: this makefile is badly written
# It currently doesn't include headers as dependencies properly
# If you change any headers, it's best to make clean before building again.

CC=gcc
SRC_DIR=src
OBJ_DIR=objs
OBJS=$(OBJ_DIR)/main.o $(OBJ_DIR)/display.o $(OBJ_DIR)/input.o $(OBJ_DIR)/platform_input.o \
	 $(OBJ_DIR)/player.o $(OBJ_DIR)/dungeon.o $(OBJ_DIR)/log.o
COMPILE_OPTS=-Wall -Wextra -Werror -Wshadow -Wconversion -Wunreachable-code

#To compile with PDCurses... \/
#LINKER_OPTS=-lpdcurses2 -lSDL2
#To compile with curses instead of PDCurses...
#LINKER_OPTS=-lcurses
#To compile with ncurses instead...
LINKER_OPTS=-lncurses

# TODO / hack: have to specify the build target manually... in the makefile...
COMPILE_OPTS += -DBUILD_CURSES

# Use this instead of CC directly
BUILD=$(CC) $(LINKER_OPTS) $(COMPILE_OPTS)

all: build/game
	$(info Running curses build by default)
	$(info Currently supported target platforms: curses)

build/game: curses
	cp game build/game

clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f game build/game
	rm -f $(OBJ_DIR)/curses/*.o

curses: $(OBJS) $(OBJ_DIR)/curses
	$(BUILD) -o game $(OBJ_DIR)/curses/*.o $(OBJ_DIR)/*.o

$(OBJ_DIR)/curses:
	mkdir $(OBJ_DIR)/curses

$(OBJ_DIR)/main.o: $(SRC_DIR)/game_layer/main.c $(SRC_DIR)/game_layer/main.h
	# "Game layer" object: output directly into $(OBJ_DIR)
	$(BUILD) -o $(OBJ_DIR)/main.o -c $(SRC_DIR)/game_layer/main.c

$(OBJ_DIR)/display.o: $(SRC_DIR)/platform_curses/display.c $(SRC_DIR)/platform_curses/display.h
	$(BUILD) -c -o $(OBJ_DIR)/display.o $(SRC_DIR)/platform_curses/display.c

$(OBJ_DIR)/platform_input.o: $(SRC_DIR)/translation_curses/platform_input.c $(SRC_DIR)/translation_curses/platform_input.h
	$(BUILD) -c -o $(OBJ_DIR)/curses/platform_input.o $(SRC_DIR)/translation_curses/platform_input.c

$(OBJ_DIR)/input.o: $(SRC_DIR)/game_layer/input.c $(SRC_DIR)/game_layer/input.h
	$(BUILD) -c -o $(OBJ_DIR)/input.o $(SRC_DIR)/game_layer/input.c

$(OBJ_DIR)/player.o: $(SRC_DIR)/game_layer/player.c $(SRC_DIR)/game_layer/player.h
	$(BUILD) -c -o $(OBJ_DIR)/player.o $(SRC_DIR)/game_layer/player.c

$(OBJ_DIR)/dungeon.o: $(SRC_DIR)/game_layer/dungeon.c $(SRC_DIR)/game_layer/dungeon.h
	$(BUILD) -c -o $(OBJ_DIR)/dungeon.o $(SRC_DIR)/game_layer/dungeon.c

$(OBJ_DIR)/log.o: $(SRC_DIR)/game_layer/log.c $(SRC_DIR)/game_layer/log.h
	$(BUILD) -c -o $(OBJ_DIR)/log.o $(SRC_DIR)/game_layer/log.c
