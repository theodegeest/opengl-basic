target := bin/main
SRCS=$(wildcard $(SRC)/*.c)
OBJS=$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))

CC := gcc
CFLAGS := -Wall
LFLAGS := -lglfw -ldl
ZIPNAME := project.zip
BIN := bin
OBJ := obj
SRC := src

$(shell mkdir -p obj bin)

all: $(target) $(BIN) $(OBJ)

debug: CFLAGS = -Wall -g
debug: clean
debug: $(target)

$(target): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(BIN)/* $(OBJ)/*

zip: 
	rm -f $(ZIPNAME)
	zip $(ZIPNAME) $(SRC)/*

cleanmake: clean
cleanmake: $(target)
