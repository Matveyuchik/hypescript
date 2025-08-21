CC=gcc
CFLAGS=-std=c11 -O2 -Wall -Wextra -Wno-unused-parameter

SRC= hypescript.c \
    src/lexer.c src/parser.c src/ast.c src/value.c src/env.c src/interp.c

INC= -Isrc

BIN=hypescript

PREFIX?=/usr
BINDIR?=$(PREFIX)/bin

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(INC) -o $@ $(SRC)

install: $(BIN)
	install -d "$(DESTDIR)$(BINDIR)"
	install -m 0755 $(BIN) "$(DESTDIR)$(BINDIR)/$(BIN)"

uninstall:
	rm -f "$(DESTDIR)$(BINDIR)/$(BIN)"

clean:
	rm -f $(BIN)

.PHONY: all clean


