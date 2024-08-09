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
	$(BIN) -f temp/test.db -a "Frodo Baggins,123 Shire Ln,40"
	$(BIN) -f temp/test.db -a "Samwise Gamgee,124 Shire Ln,40"
	$(BIN) -f temp/test.db -a "Meriadoc Brandybuck,125 Shire Ln,40"
	$(BIN) -f temp/test.db -a "Peregrin Took,126 Shire Ln,40"
	$(BIN) -f temp/test.db -l
	$(BIN) -f temp/test.db -u "Frodo Baggins,120 Shire Ln,50"
	$(BIN) -f temp/test.db -l
	$(BIN) -f temp/test.db -d "Samwise Gamgee"
	$(BIN) -f temp/test.db -l

.PHONY: clean
clean:
	rm -rf bin
	rm -rf obj
	rm -rf temp
