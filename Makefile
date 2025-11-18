# Makefile for the Neural Collision Avoidance project
# Este Makefile compila tanto o programa principal do robô quanto o programa de treinamento standalone

# Compiler and flags
CXX = g++
CXXFLAGS = -I/usr/local/Aria/include -I./include -std=c++14
CXXFLAGS += -Wno-deprecated-declarations -Wall
LDFLAGS = -L/usr/local/Aria/lib -lAria -lpthread

# Directories
SRC_DIR = src
NN_SRC_DIR = src/neuralnetwork
OBJ_DIR = build
INCLUDE_DIR = include
NN_INCLUDE_DIR = include/neuralnetwork

# Common source files (sem main functions e sem neural network)
COMMON_SRC = $(SRC_DIR)/ClassRobo.cpp $(SRC_DIR)/Colisionavoidancethread.cpp \
             $(SRC_DIR)/Laserthread.cpp $(SRC_DIR)/Sonarthread.cpp \
             $(SRC_DIR)/Wallfollowerthread.cpp
NEURAL_SRC = $(SRC_DIR)/NeuralCollisionAvoidance.cpp
NN_SRC = $(wildcard $(NN_SRC_DIR)/*.cpp)

# Object files comuns
COMMON_OBJ = $(COMMON_SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
NEURAL_OBJ = $(OBJ_DIR)/NeuralCollisionAvoidance.o
NN_OBJ = $(NN_SRC:$(NN_SRC_DIR)/%.cpp=$(OBJ_DIR)/nn_%.o)

# Object files para cada programa específico
MAIN_OBJ = $(OBJ_DIR)/main.o
MAIN_NEURAL_OBJ = $(OBJ_DIR)/main_neural.o
TRAIN_OBJ = $(OBJ_DIR)/train_network.o
TEST_OBJ = $(OBJ_DIR)/test_scenarios.o

# Targets executáveis
TARGET_ROBOT = $(OBJ_DIR)/main
TARGET_ROBOT_NEURAL = $(OBJ_DIR)/main_neural
TARGET_TRAIN = $(OBJ_DIR)/train_network
TARGET_TEST = $(OBJ_DIR)/test_scenarios

# Default target: build all programs
all: $(TARGET_ROBOT) $(TARGET_ROBOT_NEURAL) $(TARGET_TRAIN) $(TARGET_TEST)

# Robot program target (original)
robot: $(TARGET_ROBOT)

# Robot program with neural network target
robot-neural: $(TARGET_ROBOT_NEURAL)

# Training program target
train: $(TARGET_TRAIN)

# Test program target
test: $(TARGET_TEST)

# Ensure build directory exists before compiling
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Link: main robot program (original, sem neural network)
$(TARGET_ROBOT): $(MAIN_OBJ) $(COMMON_OBJ)
	@echo "Linkando programa principal do robô (versão original)..."
	$(CXX) $(MAIN_OBJ) $(COMMON_OBJ) -o $(TARGET_ROBOT) $(LDFLAGS)
	@echo "✓ Programa do robô compilado: $(TARGET_ROBOT)"

# Link: main robot program with neural network
$(TARGET_ROBOT_NEURAL): $(MAIN_NEURAL_OBJ) $(COMMON_OBJ) $(NEURAL_OBJ) $(NN_OBJ)
	@echo "Linkando programa do robô com rede neural..."
	$(CXX) $(MAIN_NEURAL_OBJ) $(COMMON_OBJ) $(NEURAL_OBJ) $(NN_OBJ) -o $(TARGET_ROBOT_NEURAL) $(LDFLAGS)
	@echo "✓ Programa do robô com neural network compilado: $(TARGET_ROBOT_NEURAL)"

# Link: training program (sem ARIA)
$(TARGET_TRAIN): $(TRAIN_OBJ) $(NN_OBJ)
	@echo "Linkando programa de treinamento..."
	$(CXX) $(TRAIN_OBJ) $(NN_OBJ) -o $(TARGET_TRAIN)
	@echo "✓ Programa de treinamento compilado: $(TARGET_TRAIN)"

# Link: test scenarios program (apenas neural network, sem ARIA)
$(TARGET_TEST): $(TEST_OBJ) $(NN_OBJ)
	@echo "Linkando programa de testes..."
	$(CXX) $(TEST_OBJ) $(NN_OBJ) -o $(TARGET_TEST)
	@echo "✓ Programa de testes compilado: $(TARGET_TEST)"

# Rule for compiling robot .cpp files into .o (object files)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@echo "Compilando $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule for compiling neural network .cpp files into .o (object files)
$(OBJ_DIR)/nn_%.o: $(NN_SRC_DIR)/%.cpp | $(OBJ_DIR)
	@echo "Compilando $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	@echo "Limpando arquivos de build..."
	rm -rf $(OBJ_DIR)
	@echo "✓ Build limpo"

# Run the robot program (original)
run: $(TARGET_ROBOT)
	@echo "Executando programa do robô (versão original)..."
	./$(TARGET_ROBOT)

# Run the robot program with neural network
run-neural: $(TARGET_ROBOT_NEURAL)
	@echo "Executando programa do robô com rede neural..."
	./$(TARGET_ROBOT_NEURAL)

# Run the training program
run-train: $(TARGET_TRAIN)
	@echo "Executando treinamento da rede neural..."
	./$(TARGET_TRAIN)

# Run the test program
run-test: $(TARGET_TEST)
	@echo "Executando testes da rede neural..."
	./$(TARGET_TEST)

# Display help information
help:
	@echo "╔════════════════════════════════════════════════════╗"
	@echo "║  Makefile - Neural Collision Avoidance             ║"
	@echo "╚════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Alvos disponíveis:"
	@echo "  all          - Compila todos os programas (padrão)"
	@echo "  robot        - Compila o programa original do robô"
	@echo "  robot-neural - Compila o programa do robô com rede neural"
	@echo "  train        - Compila o programa de treinamento"
	@echo "  test         - Compila o programa de testes"
	@echo "  run          - Compila e executa o programa original"
	@echo "  run-neural   - Compila e executa com rede neural"
	@echo "  run-train    - Compila e executa o treinamento"
	@echo "  run-test     - Compila e executa os testes"
	@echo "  clean        - Remove arquivos compilados"
	@echo "  help         - Exibe esta mensagem"
	@echo ""
	@echo "Exemplos de uso:"
	@echo "  make clean && make all    - Recompila tudo do zero"
	@echo "  make run-train            - Treina a rede neural"
	@echo "  make run-neural           - Executa robô com NN"
	@echo "  make run-test             - Executa suite de testes"
	@echo "  make              # Compila tudo"
	@echo "  make train        # Compila programa de treinamento"
	@echo "  make run-train    # Treina a rede neural"
	@echo "  make robot        # Compila programa do robô"
	@echo "  make run          # Executa o robô no simulador"
	@echo "  make clean        # Limpa build"
	@echo ""

# Phony targets
.PHONY: all robot train clean run run-train help
