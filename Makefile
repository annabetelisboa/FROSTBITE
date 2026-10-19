
CC = gcc
CFLAGS = -Wall -Wextra -g

GAME = frostbite
SRC = src/frostbite.c
LIB_SRC = cli-lib/src/keyboard.c cli-lib/src/screen.c cli-lib/src/timer.c
INCLUDES = -Icli-lib/include

all: $(GAME)

$(GAME): $(SRC) $(LIB_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC) $(LIB_SRC) -o $(GAME)

run: $(GAME)
	./$(GAME)

clean:
	rm -f $(GAME)

.PHONY: all run clean
