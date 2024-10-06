CSANITIZE = -fsanitize=undefined -fsanitize=address
CFLAGS    = -std=c99 -Wall -Wextra -Wpedantic -Werror -g -Os
CC        = cc

SOURCES = main.c editor.c cmdline.c editline.c terminal.c filebuf.c vector.c strbuf.c view.c util.c
OBJECTS = ${SOURCES:.c=.o}
BINARY  = nex

all: ${BINARY}

${BINARY}: ${OBJECTS}
	${CC} ${CSANITIZE} -o $@ $^

%.o: %.c
	${CC} ${CFLAGS} ${CSANITIZE} -c -o $@ $^

clean:
	rm ${BINARY} ${OBJECTS}
