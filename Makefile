CC = gcc
CFLAGS = -Wall -Wextra -Werror $(shell pkg-config --cflags gtk+-3.0)
LDFLAGS = $(shell pkg-config --libs gtk+-3.0)

all: task-manager

task-manager: task-manager.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f task-manager
