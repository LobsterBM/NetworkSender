all : 
	gcc -o main main.c functions.h -lz
	gcc -o test test.c functions.c -lcunit -lz



main.o : main.c
	gcc -o main main.c functions.h -lz

test: test.c
	gcc -o test test.c functions.c -lcunit -lz

clean:
	rm test
	rm main

