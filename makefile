.PHONY: main

default: main run

main:
	@gcc main.c -o program.exe -lglfw3 -lglew32 -lopengl32 -lm

run:
	@./program.exe