# ============================================================================
# Makefile — Simulador de ECU (PDS2 / UFMG) com Dear ImGui + GLFW + OpenGL3
# ============================================================================

CXX            := g++
IMGUI_DIR      := third_party/imgui
CXXFLAGS       := -std=c++11 -Wall -Wextra -I include -I $(IMGUI_DIR)
DEPFLAGS       := -MMD -MP
IMGUI_CXXFLAGS := -std=c++11 -O2 -I $(IMGUI_DIR)

ifeq ($(OS),Windows_NT)
    LDFLAGS := -lglfw3 -lopengl32 -lgdi32 -luser32 -lshell32
else
    LDFLAGS := -lglfw -lGL -ldl -lpthread
endif

SRC_DIR   := src
INC_DIR   := include
TESTS_DIR := tests
BUILD_DIR := build

TARGET   := $(BUILD_DIR)/ecu_simulator
TEST_BIN := $(BUILD_DIR)/tests_runner

# Busca automática de arquivos .cpp usando wildcard nativo (até 3 níveis em src/)
SRCS := $(wildcard $(SRC_DIR)/*.cpp) \
        $(wildcard $(SRC_DIR)/*/*.cpp) \
        $(wildcard $(SRC_DIR)/*/*/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

IMGUI_SRCS := $(wildcard $(IMGUI_DIR)/*.cpp)
IMGUI_OBJS := $(patsubst $(IMGUI_DIR)/%.cpp,$(BUILD_DIR)/imgui/%.o,$(IMGUI_SRCS))

TEST_SRCS := $(wildcard $(TESTS_DIR)/*.cpp)
TEST_OBJS := $(patsubst $(TESTS_DIR)/%.cpp,$(BUILD_DIR)/tests/%.o,$(TEST_SRCS))

PROJECT_OBJS_NO_MAIN := $(filter-out $(BUILD_DIR)/main.o,$(OBJS))
DEPS := $(OBJS:.o=.d) $(TEST_OBJS:.o=.d)

.PHONY: all run test clean docs

all: $(TARGET)

$(TARGET): $(OBJS) $(IMGUI_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(OBJS) $(IMGUI_OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/imgui/%.o: $(IMGUI_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(IMGUI_CXXFLAGS) -c $< -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_OBJS) $(PROJECT_OBJS_NO_MAIN)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(TEST_OBJS) $(PROJECT_OBJS_NO_MAIN) -o $@

$(BUILD_DIR)/tests/%.o: $(TESTS_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -I $(TESTS_DIR) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

docs:
	doxygen Doxyfile

-include $(DEPS)