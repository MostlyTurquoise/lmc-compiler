bin/lmca: src/assembler/*
	gcc src/assembler/main.c -o bin/lmca

bin/lmcx: src/runtime/*
	gcc src/runtime/main.c -o bin/lmcx

build: bin/lmca bin/lmcx

clean:
	rm -R bin/*