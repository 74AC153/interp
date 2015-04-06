CFLAGS=-Wall -Wextra -g3 --std=c99 -Wno-unused-function -D_BSD_SOURCE -D_POSIX_C_SOURCE=2
#CFLAGS=-Os

default: all
all: interp rp rpc rpod rpld rpas

interp: main.o interp.o program.o
	gcc -o interp main.o interp.o program.o

main.o: main.c
	gcc ${CFLAGS} -c main.c

interp.o: interp.c
	gcc ${CFLAGS} -c interp.c

program.o: program.c
	gcc ${CFLAGS} -c program.c

rp: rp.c interp.c rputils.c
	gcc ${CFLAGS} -ldl -o $@ $^

rpc: rpc.c rputils.c
	gcc ${CFLAGS} -o $@ $^

rpas: rpas.c rputils.c
	gcc ${CFLAGS} -o $@ $^

rpod: rpod.c rputils.c
	gcc ${CFLAGS} -o $@ $^

rpld: rpld.c rputils.c
	gcc ${CFLAGS} -o $@ $^

clean:
	rm interp rp rpc rpod rpld rpas
