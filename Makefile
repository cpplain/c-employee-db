dirs = bin obj temp

BIN = bin/edb
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
	$(BIN) -f temp/test.db -a "Johny Appleseed,1 Infinite Loop,80"
	$(BIN) -f temp/test.db -l -a "John Smith,123 Main St,40"
	$(BIN) -f temp/test.db -l -u "John Smith,1234 Main St,80"

.PHONY: clean
clean:
	rm -rf bin
	rm -rf obj
	rm -rf temp
