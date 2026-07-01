CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Iinclude

TARGET = dcsh
BUILD_DIR = build
TEST_TARGET = $(BUILD_DIR)/test_dcsh

SRC = src/main.c src/shell.c src/parser.c src/executor.c
OBJ = $(SRC:src/%.c=$(BUILD_DIR)/%.o)
TEST_SRC = tests/test_dcsh.c src/parser.c src/executor.c

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

test: $(TEST_TARGET)
	$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean test
