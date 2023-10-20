# Define directories
SRC_DIR := src
BIN_DIR := bin
BUILD := build

# Debug/Release build
BUILD := debug

# Define the compiler
CXX := g++
CXXFLAGS := -std=c++20 -pthread -pedantic
CXXFLAGS_debug := -Og -g -Wall -Wextra
CXXFLAGS_release := -O3 -DNDEBUG
CXXFLAGS += -MMD -MP $(CXXFLAGS_$(BUILD))

# Define the source files
SRC :=  $(wildcard $(SRC_DIR)/*.hpp) $(wildcard $(SRC_DIR)/*.cc)

# Define the target executable
TARGET := $(BIN_DIR)/$(BUILD)/test
TARGET_MAIN := $(SRC_DIR)/test.cc

# Include Boost
BOOST_ROOT ?= /opt/boost-1.80.0
INCLUDE = -I$(BOOST_ROOT) -I$(BOOST_ROOT)/include
LIB = -L$(BOOST_ROOT)/lib
BOOST = -lboost_thread

# Define the phony targets
.PHONY: all clean

# Define the all target
all: $(TARGET)

# Define the run target
run: $(TARGET)
	$(TARGET) localhost 1895

# Define the clean target
clean:
	rm -rf $(BIN_DIR)

# Define the target rule
$(TARGET): $(SRC) | $(BIN_DIR)/$(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LIB) -o $@ $(TARGET_MAIN) $(BOOST)

# Define the object directory rule
$(BIN_DIR)/$(BUILD):
	mkdir -p $@
