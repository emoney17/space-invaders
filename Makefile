CC = g++
CFLAGS = -Wall -Wextra -std=c++11 -pedantic
SRC = main.cpp
LIBS = -lglfw -lGLEW -lGL
# PROG = game

all: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o game
test:
	$(CC) $(CFLAGS) test.cpp $(LIBS) -o test
clean:
	$(RM) game
	$(RM) test
