CC=gcc

SRC_DIR=src
BUILD_DIR=build

.PHONY: all

#
# All
#
$(BUILD_DIR)main: always $(SRC_DIR)/main.c
	$(CC) -g -o $(BUILD_DIR)/main $(SRC_DIR)/main.c

#
# Always
#
always:
	mkdir -p $(BUILD_DIR)