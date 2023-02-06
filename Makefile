CC = g++
CFLAGS = -Wall -Wextra -std=c++11 -pedantic
SRC = main.cpp
LIBS = -lglfw -lGLEW -lGL
# PROG = game

all: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o game
clean:
	$(RM) game
