CC = gcc
CFLAGS = -Wall -g -I$(INC)

LIBS = -lm -lpthread 
SRC = src
OBJ = obj
INC = include
BIN = bin
EXE = $(BIN)/ex1
HDRS = $(INC)/passa_tempo.h
OBJS = $(OBJ)/passa_tempo.o $(OBJ)/ex1.o


$(BIN)/ex1: $(OBJS)
	$(CC) -g -o $(BIN)/ex1 $(OBJS) $(LIBS)

$(OBJ)/passa_tempo.o: $(SRC)/passa_tempo.c
	$(CC) $(CFLAGS) -c $(SRC)/passa_tempo.c -o $(OBJ)/passa_tempo.o

$(OBJ)/ex1.o: $(SRC)/main.c
	$(CC) $(CFLAGS) -c $(SRC)/main.c -o $(OBJ)/ex1.o

build: $(BIN)/ex1

clean: 
	rm -f $(EXE) $(OBJS) $(BIN)/*