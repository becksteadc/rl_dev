CC=gcc
OBJS=main.o display.o
SRC=main.c
SRC_DIR=src
OBJ_DIR=objs
COMPILE_OPTS=-Wall -Wextra -Werror -Wshadow -Wconversion -Wunreachable-code
LINKER_OPTS=-lcurses

# Use this instead of CC directly
BUILD=$(CC) $(LINKER_OPTS) $(COMPILE_OPTS)

all: build/game

build/game: game
	cp game build/game
game: $(OBJS)
	$(BUILD) -o game $(OBJ_DIR)/main.o $(OBJ_DIR)/display.o

main.o: $(SRC_DIR)/main.c
	$(BUILD) -o $(OBJ_DIR)/main.o -c $(SRC_DIR)/main.c
	

clean:
	rm -f $(OBJ_DIR)/*.o game build/game
	
display.o: $(SRC_DIR)/display.c $(SRC_DIR)/display.h
	$(BUILD) -c -o $(OBJ_DIR)/display.o $(SRC_DIR)/display.c
