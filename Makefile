#CFLAGS=-g
CFLAGS=-Os
#CFLAGS=-g -Os

interp: main.o interp.o program.o
	gcc -o interp main.o interp.o program.o

main.o: main.c
	gcc ${CFLAGS} -c main.c

interp.o: interp.c
	gcc ${CFLAGS} -c interp.c

program.o: program.c
	gcc ${CFLAGS} -c program.c

clean:
	rm interp *.o
