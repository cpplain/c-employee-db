SRC_DIR := src
BLD_DIR := build
BIN_DIR := $(BLD_DIR)/bin
DB_DIR := db

CLI_BIN := $(BIN_DIR)/dbcli
CLI_SRC_DIR := $(SRC_DIR)/cli
CLI_BLD_DIR := $(BLD_DIR)/cli
CLI_SRCS := $(wildcard $(CLI_SRC_DIR)/*.c)
CLI_OBJS := $(patsubst ${SRC_DIR}/%.c, ${BLD_DIR}/%.o, $(CLI_SRCS))

SRV_BIN := $(BIN_DIR)/dbsrv
SRV_SRC_DIR := $(SRC_DIR)/srv
SRV_BLD_DIR := $(BLD_DIR)/srv
SRV_SRCS := $(wildcard $(SRV_SRC_DIR)/*.c)
SRV_OBJS := $(patsubst ${SRC_DIR}/%.c, ${BLD_DIR}/%.o, $(SRV_SRCS))

ADDR := 127.0.0.1
PORT := 8080

default: clean build

build: build-cli build-srv

build-cli: $(CLI_BIN)

build-srv: $(SRV_BIN)

run-cli: $(CLI_BIN)
	$(CLI_BIN) -a $(ADDR) -p $(PORT) -n "Frodo Baggins,Bag End,40"

run-srv: $(SRV_BIN)
	mkdir -p $(DB_DIR)
	$(SRV_BIN) -n -f $(DB_DIR)/test.db -p $(PORT)

$(CLI_BIN): $(CLI_OBJS)
	mkdir -p $(BIN_DIR)
	gcc -o $@ $?

$(SRV_BIN): $(SRV_OBJS)
	mkdir -p $(BIN_DIR)
	gcc -o $@ $?

$(BLD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(CLI_BLD_DIR) $(SRV_BLD_DIR)
	gcc -c -o $@ $<

clean:
	rm -rf $(BLD_DIR) $(DB_DIR)

.PHONY: build build-cli build-srv clean default run-cli run-srv
