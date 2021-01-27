CC = g++
FLAG = -Wall -DUNICODE -std=c++11 -static -I./include
LIB = -lws2_32 -lwsock32

SRC_DIR = ./src
BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj

TARGET_1 = $(BUILD_DIR)/src/Server.exe
TARGET_2 = $(BUILD_DIR)/src/Client.exe

OBJ_1 = $(OBJ_DIR)/Server.obj
OBJ_2 = $(OBJ_DIR)/Client.obj

.PHONY: TARGET
TARGET: $(TARGET_1) $(TARGET_2)

$(TARGET_1): $(OBJ_1)
	-@md "$(@D)"
	$(CC) $(FLAG) $^ $(LIB) -o $@

$(TARGET_2): $(OBJ_2)
	-@md "$(@D)"
	$(CC) $(FLAG) $^ $(LIB) -o $@

$(OBJ_1): $(SRC_DIR)/Server.cpp
	-@md "$(@D)"
	$(CC) $(FLAG) -c $^ $(LIB) -o $@

$(OBJ_2): $(SRC_DIR)/Client.cpp
	-@md "$(@D)"
	$(CC) $(FLAG) -c $^ $(LIB) -o $@

.PHONY: MKDIR
MKDIR:
	-@md "$(BUILD_DIR)"
	-@md "$(OBJ_DIR)"