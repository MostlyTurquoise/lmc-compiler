bin/lmca: src/assembler/*
	gcc src/assembler/main.c -o bin/lmca

bin/lmcx: src/runtime/*
	gcc src/runtime/main.c -o bin/lmcx

bin/lmcderef: src/dereferencer/*
	gcc src/dereferencer/main.c -o bin/lmcderef

build: bin/lmca bin/lmcx bin/lmcderef

clean:
	rm -R bin/*