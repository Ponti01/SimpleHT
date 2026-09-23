CC = gcc
CFLAGS = -g -Wall -Wextra -Wpedantic -Wconversion

APP_DIR = app
SRC_DIR = src
TEST_DIR = test
BIN_DIR = bin
BUILD_DIR = build
INCLUDE_DIR = include

UNITY_DIR = unity
UNITY_SRC = $(UNITY_DIR)/unity.c

.PHONY: all clean test FORCE

all: $(BIN_DIR)/hash_table $(BIN_DIR)/hash_table_test

# --- Oggetto del main (app/) ---
$(BUILD_DIR)/%.o: $(APP_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCLUDE_DIR)

# --- Oggetti dal codice sorgente principale (src/) ---
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCLUDE_DIR)

# --- Oggetto di Unity (framework di test) ---
$(BUILD_DIR)/unity.o: $(UNITY_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -I$(UNITY_DIR)

# --- Oggetti dai test ---
$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -I$(SRC_DIR) -I$(INCLUDE_DIR) -I$(UNITY_DIR)

# --- Eseguibile principale ---
$(BIN_DIR)/hash_table: $(BUILD_DIR)/hash_table.o $(BUILD_DIR)/hash_utils.o $(BUILD_DIR)/main.o
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# --- Eseguibile dei test ---
$(BIN_DIR)/hash_table_test: $(BUILD_DIR)/hash_table.o $(BUILD_DIR)/hash_utils.o $(BUILD_DIR)/hash_table_test.o $(BUILD_DIR)/unity.o
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# --- make test: esegue la suite di test ---
test: $(BIN_DIR)/hash_table_test
	./$(BIN_DIR)/hash_table_test

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
