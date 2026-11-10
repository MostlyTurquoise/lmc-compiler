bin/assembler: src/assembler/*
	gcc src/assembler/main.c -o bin/assembler

build: bin/assembler