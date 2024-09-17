target := bin/main
SRCS = $(shell find $(SRC)/ -name "*.c")
OBJS = $(patsubst %.c, $(OBJ)/%.o, $(SRCS))

CC := gcc
CFLAGS := -Wall
# CFLAGS := -Wall -O3
LFLAGS := -lglfw -ldl -lm -lcglm -lGL
ZIPNAME := project.zip
BIN := bin
OBJ := obj
SRC := src

$(shell mkdir -p obj bin)

all: $(target)

debug: CFLAGS = -Wall -g -O0
debug: clean
debug: $(target)

vsync: CFLAGS = -Wall -DVSYNC
vsync: $(target)

$(target): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

$(OBJ)/%.o: %.c
	@mkdir -p $(dir $@)  # Create the necessary directories
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BIN)/* $(OBJ)/*

zip: 
	rm -f $(ZIPNAME)
	zip $(ZIPNAME) $(SRC)/*

release: CFLAGS = -Wall -O3
release: clean
release: $(target)

dev: all
