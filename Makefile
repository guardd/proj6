CC = gcc
CFLAGS = -Wall -Wextra -Werror

all: task-manager

task-manager: task-manager.c
	$(CC) $(CFLAGS) -o task-manager task-manager.c

clean:
	rm -f task-manager
