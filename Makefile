run: compile
	./build/snake

all: clean run

compile:
	gcc -lSDL2 src/snake.c -o build/snake

clean:
	rm -f src/*.o build/snake
