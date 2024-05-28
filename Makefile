SHELL = /bin/sh

dirs = bin obj temp

BIN = bin/dbcli
SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c, obj/%.o, $(SRCS))

.PHONY: all
all: test

$(BIN): $(OBJS) | bin
	gcc -o $@ $?

obj/%.o: src/%.c | obj
	gcc -c -o $@ $<

$(dirs):
	mkdir -p $@

.PHONY: test
test: clean $(BIN) | temp
	$(BIN) -n -f temp/test.db
	$(BIN) -f temp/test.db

.PHONY: clean
clean:
	rm -rf bin
	rm -rf obj
	rm -rf temp
