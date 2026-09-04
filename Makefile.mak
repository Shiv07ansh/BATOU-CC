# ==============================================================================
# BATOU-CC: Bare-Metal Neural Network C Compiler Makefile
# ==============================================================================

# Compilers & Flags
CXX      := g++
CC       := gcc
CXXFLAGS := -O2 -std=c++17 -Iinclude
CFLAGS   := -O2 -Iruntime

# Directory Structure
BIN_DIR  := bin
GEN_DIR  := generated
SRC_DIR  := src
MDL_DIR  := models
RUN_DIR  := runtime

# Targets & File Paths
COMPILER_BIN := $(BIN_DIR)/batou-cc
RUNNER_BIN   := $(BIN_DIR)/test_runner
MODEL_JSON   := $(MDL_DIR)/model.json
COMPILED_C   := $(GEN_DIR)/model_compiled.c

# Phony Targets (commands that aren't physical files)
.PHONY: all clean run compiler test

# Default rule: Build the runner executable
all: $(RUNNER_BIN)

# Ensure output directories exist
$(BIN_DIR) $(GEN_DIR):
	@mkdir -p $@

# Step 1: Compile the C++ AOT Compiler
$(COMPILER_BIN): $(SRC_DIR)/compiler.cpp | $(BIN_DIR)
	@echo "[CXX] Compiling BATOU-CC frontend..."
	$(CXX) $(CXXFLAGS) $< -o $@

# Step 2: Generate static C code from the JSON model
$(COMPILED_C): $(COMPILER_BIN) $(MODEL_JSON) | $(GEN_DIR)
	@echo "[BATOU-CC] Emitting bare-metal C code..."
	./$(COMPILER_BIN) $(MODEL_JSON) -o $(COMPILED_C)

# Step 3: Link generated C code with runtime kernels and test runner
$(RUNNER_BIN): $(COMPILED_C) $(RUN_DIR)/kernels.c $(RUN_DIR)/runner.c | $(BIN_DIR)
	@echo "[GCC] Building bare-metal runtime test runner..."
	$(CC) $(CFLAGS) $(COMPILED_C) $(RUN_DIR)/kernels.c $(RUN_DIR)/runner.c -o $@

# Convenience target to build and immediately run inference
run: $(RUNNER_BIN)
	@echo "[EXEC] Running bare-metal model inference..."
	@./$(RUNNER_BIN)

# Flush all generated build artifacts and binaries
clean:
	@echo "[CLEAN] Flushing bin/ and generated/ directories..."
	@rm -rf $(BIN_DIR) $(GEN_DIR)