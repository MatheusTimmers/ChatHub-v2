CXX       := g++
CXXFLAGS  := -std=c++11 -Wall -Iinclude

SRC_DIR   := src
OBJ_DIR   := build
BIN_DIR   := bin

SRCS      := $(shell find . -maxdepth 1 -name 'main.cpp') \
             $(shell find $(SRC_DIR) -type f -name '*.cpp')
OBJS      := $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

TARGET    := $(BIN_DIR)/usernet

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

run: all
	$(TARGET)
