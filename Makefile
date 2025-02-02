SRC_DIR = src
BLD_DIR = build
BIN_DIR = bin
DB_DIR = db

SRV_BIN = $(BIN_DIR)/dbsrv
SRCS = $(wildcard $(SRC_DIR)/**/*.c)
OBJS = $(patsubst ${SRC_DIR}/%.c, ${BLD_DIR}/%.o, $(SRCS))

default: clean build

build: $(SRV_BIN)

$(SRV_BIN): $(OBJS)
	mkdir -p $(BIN_DIR)
	gcc -o $@ $?

$(BLD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BLD_DIR)/srv
	gcc -c -o $@ $<

run: $(SRV_BIN)
	mkdir -p $(DB_DIR)
	$(SRV_BIN) -n -f $(DB_DIR)/test.db

clean:
	rm -rf $(BIN_DIR) $(BLD_DIR) $(DB_DIR)

.PHONY: build clean default run
