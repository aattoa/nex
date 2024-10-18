MAKEFLAGS += --jobs=8

CSANITIZE = -fsanitize=undefined -fsanitize=address
CFLAGS    = -std=c99 -Wall -Wextra -Wpedantic -Werror -g -Os

SOURCES = main.c editor.c cmdline.c visual.c registers.c settings.c terminal.c filebuf.c vector.c strbuf.c view.c util.c
OBJECTS = ${SOURCES:.c=.o}
BINARY  = nex

all: ${BINARY}

${BINARY}: ${OBJECTS}
	$(info Linking ${BINARY})
	@cc ${CSANITIZE} -o $@ $^

%.o: %.c
	$(info Compiling $^)
	@cc ${CFLAGS} ${CSANITIZE} -c $^ -o $@

clean:
	@rm ${BINARY} ${OBJECTS} 2>/dev/null
