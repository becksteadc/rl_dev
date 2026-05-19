CC=gcc
OBJS=main.o
SRC=main.c
SRC_DIR=src
OBJ_DIR=objs

all: game

game: $(OBJS)
	$(CC) -o game $(OBJ_DIR)/main.o

main.o: $(SRC_DIR)/main.c
	$(CC) -o $(OBJ_DIR)/main.o -c $(SRC_DIR)/main.c
	

clean:
	rm -f $(OBJ_DIR)/*.o game
