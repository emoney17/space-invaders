CC = g++
SRC = game.cpp
LIB = -lglfw -lGLEW -lGL
PROG = game

game:
	$(CC) $(SRC) $(LIB) -o $(PROG)
clean:
	rm -f $(PROG)
