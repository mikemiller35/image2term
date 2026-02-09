# Compiler settings
CXX := clang++
CXXFLAGS := -std=c++23 -Wall -Wextra -I./include

# Platform/Arch detection
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Directories
SRC_DIR := src
BUILD_DIR := build
INCLUDE_DIR := include

# Source files
LIB_SRC := $(SRC_DIR)/image2term.cpp
MAIN_SRC := $(SRC_DIR)/main.cpp
SRCS := $(MAIN_SRC) $(LIB_SRC)
TEST_SRC := $(SRC_DIR)/test_image2term.cpp
TARGET := image2term
TEST_TARGET := test_image2term

# Platform-specific settings
ifeq ($(UNAME_S),Darwin)
    PLATFORM := darwin
else ifeq ($(UNAME_S),Linux)
    PLATFORM := linux
endif

ifeq ($(UNAME_M),arm64)
    ARCH := arm64
else ifeq ($(UNAME_M),aarch64)
    ARCH := arm64
else
    ARCH := amd64
endif

# Default build
all: $(BUILD_DIR)/$(TARGET)

# Alias for all - Note: 'build' conflicts with directory name, use 'all' instead
# build: all

# Native build
$(BUILD_DIR)/$(TARGET): $(MAIN_SRC) $(LIB_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -O2 $(MAIN_SRC) $(LIB_SRC) -o $@

# Test build and run
test: $(BUILD_DIR)/$(TEST_TARGET)
	./$(BUILD_DIR)/$(TEST_TARGET)

$(BUILD_DIR)/$(TEST_TARGET): $(TEST_SRC) $(LIB_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -g $(TEST_SRC) $(LIB_SRC) -o $@

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: $(BUILD_DIR)/$(TARGET)-debug

$(BUILD_DIR)/$(TARGET)-debug: $(SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $@

# Release build (with optimizations)
release: CXXFLAGS += -O3 -DNDEBUG
release: $(BUILD_DIR)/$(TARGET)

# Cross-compilation targets
darwin-arm64: $(BUILD_DIR)/$(TARGET)-darwin-arm64
darwin-amd64: $(BUILD_DIR)/$(TARGET)-darwin-amd64
linux-arm64: $(BUILD_DIR)/$(TARGET)-linux-arm64
linux-amd64: $(BUILD_DIR)/$(TARGET)-linux-amd64

$(BUILD_DIR)/$(TARGET)-darwin-arm64: $(SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -target arm64-apple-darwin -O2 $(SRCS) -o $@

$(BUILD_DIR)/$(TARGET)-darwin-amd64: $(SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -target x86_64-apple-darwin -O2 $(SRCS) -o $@

$(BUILD_DIR)/$(TARGET)-linux-arm64: $(SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -target aarch64-linux-gnu -O2 $(SRCS) -o $@

$(BUILD_DIR)/$(TARGET)-linux-amd64: $(SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -target x86_64-linux-gnu -O2 $(SRCS) -o $@

# Build all cross-compilation targets
cross: darwin-arm64 darwin-amd64 linux-arm64 linux-amd64

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean debug release test cross darwin-arm64 darwin-amd64 linux-arm64 linux-amd64
