# ============================================================================
# Makefile — Simulador de ECU (PDS2 / UFMG)
# Projeto alvo: Linux; compatível com MSYS2/MinGW no Windows (usar `make` do
# MSYS2 ou `mingw32-make`).
# ============================================================================

# --- Variáveis de topo -------------------------------------------------------
# Compilador e flags obrigatórias do projeto (C++11 estrito, warnings on).
CXX        := g++
CXXFLAGS   := -std=c++11 -Wall -Wextra -I include
# -MMD / -MP geram arquivos .d com as dependências de headers por .o,
# permitindo recompilação incremental correta quando um .hpp muda.
DEPFLAGS   := -MMD -MP

# Diretórios principais.
SRC_DIR    := src
INC_DIR    := include
TESTS_DIR  := tests
BUILD_DIR  := build

# Nomes dos binários finais.
TARGET     := $(BUILD_DIR)/ecu_simulator
TEST_BIN   := $(BUILD_DIR)/tests_runner

# --- Descoberta automática de fontes ----------------------------------------
# Encontra todos os .cpp em src/ e subpastas e mapeia para .o em build/,
# preservando a estrutura de diretórios.
SRCS       := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS       := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Fontes dos testes (um TEST_CASE por arquivo + test_main.cpp).
TEST_SRCS  := $(shell find $(TESTS_DIR) -maxdepth 1 -name '*.cpp')
TEST_OBJS  := $(patsubst $(TESTS_DIR)/%.cpp,$(BUILD_DIR)/tests/%.o,$(TEST_SRCS))

# Para o alvo `test`, o runner usa os .o do projeto EXCETO main.cpp (o doctest
# já fornece o `main` via DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN).
PROJECT_OBJS_NO_MAIN := $(filter-out $(BUILD_DIR)/main.o,$(OBJS))

# Arquivos de dependência gerados por -MMD (incluídos ao final).
DEPS := $(OBJS:.o=.d) $(TEST_OBJS:.o=.d)

# --- Alvo padrão -------------------------------------------------------------
# `make`: compila o executável do simulador em build/ecu_simulator.
.PHONY: all
all: $(TARGET)

# Linka o executável do simulador a partir de todos os .o do projeto.
$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

# --- Regra genérica para compilar src/**/*.cpp em build/**/*.o --------------
# A diretiva `| $(dir $@)` garante que o diretório de destino existe antes
# da compilação (preserva a estrutura de subpastas de src/).
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

# --- Testes ------------------------------------------------------------------
# `make test`: compila os .cpp de tests/ junto com os .o do projeto (sem
# main.cpp), gera build/tests_runner e o executa.
.PHONY: test
test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_OBJS) $(PROJECT_OBJS_NO_MAIN) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(TEST_OBJS) $(PROJECT_OBJS_NO_MAIN) -o $@

# Regra para compilar os .cpp de tests/ em build/tests/*.o.
$(BUILD_DIR)/tests/%.o: $(TESTS_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -I $(TESTS_DIR) -c $< -o $@

# --- Criação do diretório build/ --------------------------------------------
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# --- Alvos auxiliares --------------------------------------------------------
# `make run`: compila e executa o simulador.
.PHONY: run
run: $(TARGET)
	./$(TARGET)

# `make clean`: remove todo o conteúdo gerado em build/.
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

# `make docs`: gera documentação Doxygen em build/docs/.
.PHONY: docs
docs:
	doxygen Doxyfile

# --- Inclusão dos arquivos de dependência -----------------------------------
# O `-include` ignora silenciosamente se os .d ainda não existirem (primeira
# compilação).
-include $(DEPS)
