# Compiler and flags
CC = gcc
CFLAGS = -Wall -Werror -g -Og -I./include
LDFLAGS = 

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = build
BIN_DIR = bin
TEST_DIR = tests

# Target executable
TARGET = $(BIN_DIR)/customShell

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Header files (for dependency tracking)
DEPS = $(wildcard $(INC_DIR)/*.h)

# Phony targets
.PHONY: all clean test directories help

# Default target
all: directories $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Build successful! Executable is at $@"

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Create necessary directories
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Cleaned build artifacts."

# Run tests (placeholder - can be expanded)
test: all
	@echo "Running basic tests..."
	./$(TARGET) < $(TEST_DIR)/test_input.txt || echo "No automated tests yet."

# Help message
help:
	@echo "Available targets:"
	@echo "  all     : Build the project (default)"
	@echo "  clean   : Remove build artifacts"
	@echo "  test    : Run tests"
	@echo "  help    : Show this help message"
