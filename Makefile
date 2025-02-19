compile: build run
build:
	g++ main.cpp -o main -lSDL3 -lSDL3_image -lSDL3_ttf
run:
	./main.exe