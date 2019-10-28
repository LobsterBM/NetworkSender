all : 
	gcc -o sender ./src/client.c ./src/functions.h -lz -std=c99
	gcc -o test ./tests/test.c ./src/functions.c -lcunit -lz -std=c99



client.o : client.c
	gcc -o client ./src/client.c ./src/functions.h -lz -std=c99

test: test.c
	gcc -o test ./tests/test.c ./src/functions.c -lcunit -lz -std=c99

clean:
	rm sender
	rm test
	

