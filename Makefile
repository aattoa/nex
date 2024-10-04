CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Werror -g -Os
CC     = cc

SOURCES = main.c editor.c terminal.c filebuf.c vector.c strbuf.c buffer.c view.c util.c
OBJECTS = ${SOURCES:.c=.o}
BINARY  = nex

all: ${BINARY}

${BINARY}: ${OBJECTS}
	${CC} -o $@ $^

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $^

clean:
	rm ${BINARY} ${OBJECTS}
