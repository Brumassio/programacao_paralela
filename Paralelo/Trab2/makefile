# Makefile para compilar todos os arquivos .c em um único executável

# Compilador
CC = gcc

# Opções do compilador
CFLAGS = -O3

# Nome do executável
TARGET = atax_time

# Lista de arquivos fonte (todos os .c no diretório atual)
SRCS = $(wildcard *.c)

# Parâmetros de execução
DATASET_SIZE ?= -DEXTRALARGE_DATASET
ARGS = $(DATASET_SIZE) -DPOLYBENCH_TIME

# Regra padrão: construir o executável
all: $(TARGET)

# Regra para construir o executável
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) $(ARGS) -o $(TARGET)

# Regra para limpar o arquivo executável
clean:
	rm -f $(TARGET)

# Marcar as regras como "falsas" (phony) para evitar conflitos com arquivos de mesmo nome
.PHONY: all clean
