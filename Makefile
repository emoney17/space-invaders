CC = g++
CFLAGS = -Wall -Wextra -std=c++11 -pedantic
SRC = main.cpp
LIBS = -lglfw -lGLEW -lGL

all: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o game
