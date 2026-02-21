# Compiler and flags
CXX := g++
NVCC := nvcc
CXXFLAGS := -O3 -std=c++17 -Wall -Wextra -Iinclude
NVCCFLAGS := -O3 -std=c++17 -Iinclude -Xcompiler "-Wall -Wextra" -arch=sm_52

# Target executable
TARGET := sha256_cracker

# Source files
CPP_SRCS := $(wildcard src/*.cpp)
CU_SRCS  := $(wildcard src/*.cu)

# Object files
OBJ_DIR  := build
CPP_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(CPP_SRCS))
CU_OBJS  := $(patsubst src/%.cu,$(OBJ_DIR)/%_cuda.o,$(CU_SRCS))

# Default target
all: $(TARGET)

# Link the final executable
$(TARGET): $(CPP_OBJS) $(CU_OBJS)
	$(NVCC) $(NVCCFLAGS) $^ -o $@ -lpthread -lcudart

# Compile C++ source files
$(OBJ_DIR)/%.o: src/%.cpp | $(OBJ_DIR)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile CUDA source files
$(OBJ_DIR)/%_cuda.o: src/%.cu | $(OBJ_DIR)
	@mkdir -p $(@D)
	$(NVCC) $(NVCCFLAGS) -dc $< -o $@

# Create build directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Test targets
.PHONY: test test-short test-long benchmark

# Run short password tests (3-4 chars)
test-short: $(TARGET)
	@echo "=== Running short password tests (3-4 chars) ==="
	@if [ -f test_short_passwords.sh ]; then \
		./test_short_passwords.sh; \
	else \
		echo "Test script not found. Creating it..."; \
		echo '#!/bin/bash' > test_short_passwords.sh; \
		echo 'echo "Testing 3-char password (abc)..."' >> test_short_passwords.sh; \
		echo 'HASH=$$(echo -n "abc" | sha256sum | cut -d" " -f1)' >> test_short_passwords.sh; \
		echo './$(TARGET) --target $$HASH --mode gpu --min 3 --max 3 --alphabet "abc"' >> test_short_passwords.sh; \
		chmod +x test_short_passwords.sh; \
		./test_short_passwords.sh; \
	fi

# Run long password tests (5-8 chars)
test-long: $(TARGET)
	@echo "=== Running long password tests (5-8 chars) ==="
	@if [ -f test_long_passwords.sh ]; then \
		./test_long_passwords.sh; \
	else \
		echo "Test script not found. Creating it..."; \
		echo '#!/bin/bash' > test_long_passwords.sh; \
		echo 'echo "Testing 5-char password (hello)..."' >> test_long_passwords.sh; \
		echo 'HASH=$$(echo -n "hello" | sha256sum | cut -d" " -f1)' >> test_long_passwords.sh; \
		echo './$(TARGET) --target $$HASH --mode gpu --min 5 --max 5 --alphabet "helo"' >> test_long_passwords.sh; \
		chmod +x test_long_passwords.sh; \
		./test_long_passwords.sh; \
	fi

# Run all tests
test: test-short test-long

# Run benchmark
benchmark: $(TARGET)
	@echo "=== Running benchmark ==="
	@HASH=$$(echo -n "benchmark" | sha256sum | cut -d" " -f1); \
	./$(TARGET) --target $$HASH --mode gpu --min 6 --max 8 --benchmark

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(TARGET) test_*.sh

# Run the program
run: $(TARGET)
	@echo "Usage: ./$(TARGET) --target <hash> --mode [cpu|gpu] [options]"
	@echo "Run './$(TARGET) --help' for more options"