dirs = bin obj test

BIN = bin/edb
SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c, obj/%.o, $(SRCS))

.PHONY: all
all: clean build

.PHONY: build
build: $(BIN)

$(BIN): $(OBJS) | bin
	gcc -o $@ $?

obj/%.o: src/%.c | obj
	gcc -c -o $@ $<

$(dirs):
	mkdir -p $@

.PHONY: test
test: clean $(BIN) | test
	$(BIN) -n -f test/test.db
	$(BIN) -f test/test.db -a "Frodo Baggins,123 Shire Ln,40"
	$(BIN) -f test/test.db -a "Samwise Gamgee,124 Shire Ln,40"
	$(BIN) -f test/test.db -a "Meriadoc Brandybuck,125 Shire Ln,40"
	$(BIN) -f test/test.db -a "Peregrin Took,126 Shire Ln,40"
	$(BIN) -f test/test.db -l
	$(BIN) -f test/test.db -u "Frodo Baggins,120 Shire Ln,50"
	$(BIN) -f test/test.db -l
	$(BIN) -f test/test.db -d "Samwise Gamgee"
	$(BIN) -f test/test.db -l

.PHONY: clean
clean:
	rm -rf bin
	rm -rf obj
	rm -rf test
