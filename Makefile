CC = gcc
CFLAGS = -Wall -Wextra -pthread $(shell pkg-config --cflags libmongoc-1.0 libbson-1.0)
LDFLAGS = $(shell pkg-config --libs libmongoc-1.0 libbson-1.0) -ljson-c -lcsv

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Lista de arquivos fonte
SRCS = $(wildcard $(SRC_DIR)/*.c) \
       $(wildcard $(SRC_DIR)/config/*.c) \
       $(wildcard $(SRC_DIR)/data/*.c) \
       $(wildcard $(SRC_DIR)/mongodb/*.c) \
       $(wildcard $(SRC_DIR)/csv/*.c) \
       $(wildcard $(SRC_DIR)/utils/*.c)

# Lista de arquivos objeto
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Nome do executável
TARGET = $(BIN_DIR)/csv_to_mongo

# Regra principal
all: directories $(TARGET)

# Cria os diretórios necessários
directories:
	@mkdir -p $(OBJ_DIR)/config
	@mkdir -p $(OBJ_DIR)/data
	@mkdir -p $(OBJ_DIR)/mongodb
	@mkdir -p $(OBJ_DIR)/csv
	@mkdir -p $(OBJ_DIR)/utils
	@mkdir -p $(BIN_DIR)

# Compila o executável
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compila os arquivos objeto
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Limpa os arquivos gerados
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean directories 