BIN_DIR = bin
BLD_DIR = build
DB_DIR = db
SRC_DIR = src

SRV_BIN = $(BIN_DIR)/dbsrv
SRV_SRCS = $(wildcard src/*c) $(wildcard src/srv/*.c)
SRV_OBJS = $(patsubst ${SRC_DIR}/%.c, ${BLD_DIR}/%.o, $(SRV_SRCS))

default: clean build

build: $(SRV_BIN)

$(SRV_BIN): $(SRV_OBJS)
	mkdir -p bin
	gcc -o $@ $?

$(BLD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p build/srv
	gcc -c -o $@ $<

run: $(SRV_BIN)
	mkdir -p $(DB_DIR)
	$(SRV_BIN) -n -f $(DB_DIR)/test.db

clean:
	rm -rf $(BIN_DIR) $(BLD_DIR) $(DB_DIR)

.PHONY: build clean default run
