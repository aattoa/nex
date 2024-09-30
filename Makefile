CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Werror -Os
CC     = cc

SOURCES = main.c view.c buffer.c
OBJECTS = ${SOURCES:.c=.o}
BINARY  = nex

all: ${BINARY}

${BINARY}: ${OBJECTS}
	${CC} -o $@ $^

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $^

clean:
	rm ${BINARY} ${OBJECTS}
