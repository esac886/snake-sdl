run: compile
	./build/snake

all: run

compile:
	gcc -lSDL2 -lSDL2_ttf src/snake.c -g -o build/snake

clean:
	rm -f src/*.o build/snake
