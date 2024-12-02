#
# CS252 Project 6 Group 27 - Task Manager
#

CC = gcc
CFLAGS = -Wall -Wextra -Werror $(shell pkg-config --cflags gtk+-3.0)
LDFLAGS = $(shell pkg-config --libs gtk+-3.0)

all: task-manager-app

system_info.o: system_info.c system_info.h
	$(CC) $(CFLAGS) -c system_info.c

processes.o: processes.c processes.h
	$(CC) $(CFLAGS) -c processes.c

resources.o: resources.c resources.h
	$(CC) $(CFLAGS) -c resources.c

file_systems.o: file_systems.c file_systems.h
	$(CC) $(CFLAGS) -c file_systems.c

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

task-manager-app: system_info.o processes.o resources.o file_systems.o main.o
	$(CC) $(CFLAGS) -o task-manager-app system_info.o processes.o resources.o file_systems.o main.o $(LDFLAGS) -lm

clean:
	rm -f task-manager task-manager-app *.o ./tmp/*
