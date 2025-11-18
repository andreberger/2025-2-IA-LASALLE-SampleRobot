/**
 * @file test_scenarios.cpp
 * @brief Testes unitários e de integração para a rede neural
 * 
 * Este arquivo demonstra como testar a rede neural com diversos cenários
 * antes de executar no robô real/simulado.
 * 
 * Compile e execute:
 *   g++ -I./include -std=c++14 test_scenarios.cpp
 *       src/neuralnetwork/Layer.cpp src/neuralnetwork/NeuralNetwork.cpp
 *       -o build/test_scenarios
 *   ./build/test_scenarios
 */

#include "neuralnetwork/NeuralNetwork.h"
#include "neuralnetwork/ActivationFunction.h"
#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>

// Função auxiliar para comparar doubles
bool approximately_equal(double a, double b, double epsilon = 0.1) {
    return std::abs(a - b) < epsilon;
}

// Interpreta saída da rede
std::string interpret_action(double output) {
    if (output >= 0.50 && output < 0.56) return "DIREITA";
    if (output >= 0.56 && output < 0.62) return "ESQUERDA";
    if (output >= 0.62 && output < 0.68) return "FRENTE";
    if (output >= 0.68 && output < 0.74) return "TRAS";
    if (output >= 0.74 && output < 0.80) return "PARAR";
    return "INDEFINIDO";
}

// Teste 1: Verificar criação da rede
bool test_network_creation() {
    std::cout << "\n[TEST 1] Criação da rede neural..." << std::endl;
    
    try {
        NeuralNetwork network(4, 1, 0.3, 0.9);
        network.addHiddenLayer(5, std::make_shared<SigmoidActivation>());
        network.finalize(std::make_shared<SigmoidActivation>());
        
        std::cout << "  ✓ Rede criada com sucesso" << std::endl;
        std::cout << network.getArchitectureInfo() << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "  ✗ Erro: " << e.what() << std::endl;
        return false;
    }
}

// Teste 2: Forward propagation básico
bool test_forward_propagation() {
    std::cout << "\n[TEST 2] Forward propagation..." << std::endl;
    
    try {
        NeuralNetwork network(4, 1, 0.3, 0.9);
        network.addHiddenLayer(5, std::make_shared<SigmoidActivation>());
        network.finalize(std::make_shared<SigmoidActivation>());
        
        std::vector<double> input = {1.0, 0.0, 1.0, 0.0};
        std::vector<double> output = network.predict(input);
        
        assert(output.size() == 1);
        assert(output[0] >= 0.0 && output[0] <= 1.0);  // Sigmoid range
        
        std::cout << "  ✓ Forward propagation OK" << std::endl;
        std::cout << "    Input: [1, 0, 1, 0] -> Output: " << output[0] << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "  ✗ Erro: " << e.what() << std::endl;
        return false;
    }
}

// Teste 3: Treinamento básico
bool test_training() {
    std::cout << "\n[TEST 3] Treinamento básico..." << std::endl;
    
    try {
        NeuralNetwork network(4, 1, 0.3, 0.9);
        network.addHiddenLayer(5, std::make_shared<SigmoidActivation>());
        network.finalize(std::make_shared<SigmoidActivation>());
        
        // Treinar com um padrão simples
        std::vector<std::vector<double>> inputs = {{1, 0, 0, 0}};
        std::vector<std::vector<double>> targets = {{0.53}};  // DIREITA
        
        double initial_error = network.validate(inputs, targets, false);
        
        network.trainBatch(inputs, targets, 10000, 0.01, false);
        
        double final_error = network.validate(inputs, targets, false);
        
        std::cout << "  Erro inicial: " << initial_error << std::endl;
        std::cout << "  Erro final:   " << final_error << std::endl;
        
        if (final_error < initial_error) {
            std::cout << "  ✓ Rede aprendeu (erro diminuiu)" << std::endl;
            return true;
        } else {
            std::cout << "  ⚠ Rede não melhorou significativamente" << std::endl;
            return false;
        }
        
    } catch (const std::exception& e) {
        std::cout << "  ✗ Erro: " << e.what() << std::endl;
        return false;
    }
}

// Teste 4: Cenários específicos de navegação
bool test_navigation_scenarios() {
    std::cout << "\n[TEST 4] Cenários de navegação..." << std::endl;
    
    try {
        // Criar e treinar rede
        NeuralNetwork network(4, 1, 0.3, 0.9);
        network.addHiddenLayer(5, std::make_shared<SigmoidActivation>());
        network.finalize(std::make_shared<SigmoidActivation>());
        
        // Dataset completo
        std::vector<std::vector<double>> inputs = {
            {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1},
            {0, 0, 1, 1}, {1, 1, 0, 0}, {0, 1, 1, 0}, {1, 0, 0, 1},
            {1, 0, 1, 0}, {0, 1, 0, 1}, {0, 1, 1, 1}, {1, 0, 1, 1},
            {1, 1, 0, 1}, {1, 1, 1, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}
        };
        
        std::vector<std::vector<double>> targets = {
            {0.53}, {0.59}, {0.65}, {0.71},
            {0.65}, {0.53}, {0.65}, {0.53},
            {0.65}, {0.59}, {0.65}, {0.65},
            {0.53}, {0.65}, {0.65}, {0.77}
        };
        
        std::cout << "  Treinando com 16 padrões..." << std::endl;
        network.trainBatch(inputs, targets, 50000, 0.004, false);
        
        // Testar cenários críticos
        struct TestCase {
            std::vector<double> input;
            std::string expected_action;
            std::string description;
        };
        
        std::vector<TestCase> test_cases = {
            {{0, 0, 1, 0}, "FRENTE", "Corredor estreito"},
            {{1, 1, 0, 0}, "DIREITA", "Cruzamento"},
            {{1, 1, 1, 1}, "FRENTE", "Espaço aberto"},
            {{0, 0, 0, 0}, "PARAR", "Bloqueio total"}
        };
        
        int passed = 0;
        for (const auto& test : test_cases) {
            auto output = network.predict(test.input);
            std::string action = interpret_action(output[0]);
            
            std::cout << "  " << test.description << ": ";
            std::cout << action;
            
            if (action == test.expected_action) {
                std::cout << " ✓" << std::endl;
                passed++;
            } else {
                std::cout << " ✗ (esperado: " << test.expected_action << ")" << std::endl;
            }
        }
        
        std::cout << "  Cenários passados: " << passed << "/" << test_cases.size() << std::endl;
        return passed == static_cast<int>(test_cases.size());
        
    } catch (const std::exception& e) {
        std::cout << "  ✗ Erro: " << e.what() << std::endl;
        return false;
    }
}

// Teste 5: Consistência de decisões
bool test_decision_consistency() {
    std::cout << "\n[TEST 5] Consistência de decisões..." << std::endl;
    
    try {
        NeuralNetwork network(4, 1, 0.3, 0.9);
        network.addHiddenLayer(5, std::make_shared<SigmoidActivation>());
        network.finalize(std::make_shared<SigmoidActivation>());
        
        // Treinar
        std::vector<std::vector<double>> inputs = {{1, 0, 1, 0}};
        std::vector<std::vector<double>> targets = {{0.65}};
        network.trainBatch(inputs, targets, 10000, 0.01, false);
        
        // Testar múltiplas vezes com mesma entrada
        std::vector<double> test_input = {1, 0, 1, 0};
        std::vector<double> first_output = network.predict(test_input);
        
        bool consistent = true;
        for (int i = 0; i < 10; i++) {
            auto output = network.predict(test_input);
            if (!approximately_equal(output[0], first_output[0], 0.0001)) {
                consistent = false;
                break;
            }
        }
        
        if (consistent) {
            std::cout << "  ✓ Decisões consistentes para mesma entrada" << std::endl;
            return true;
        } else {
            std::cout << "  ✗ Decisões inconsistentes" << std::endl;
            return false;
        }
        
    } catch (const std::exception& e) {
        std::cout << "  ✗ Erro: " << e.what() << std::endl;
        return false;
    }
}

// Teste 6: Salvamento de pesos (básico)
bool test_weight_saving() {
    std::cout << "\n[TEST 6] Salvamento de pesos..." << std::endl;
    
    try {
        NeuralNetwork network(4, 1, 0.3, 0.9);
        network.addHiddenLayer(5, std::make_shared<SigmoidActivation>());
        network.finalize(std::make_shared<SigmoidActivation>());
        
        bool saved = network.saveWeights("test_weights_temp.json");
        
        if (saved) {
            std::cout << "  ✓ Pesos salvos com sucesso" << std::endl;
            // Limpar arquivo de teste
            std::remove("test_weights_temp.json");
            return true;
        } else {
            std::cout << "  ✗ Falha ao salvar pesos" << std::endl;
            return false;
        }
        
    } catch (const std::exception& e) {
        std::cout << "  ✗ Erro: " << e.what() << std::endl;
        return false;
    }
}

// Main
int main() {
    std::cout << "╔════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║   TESTES DA REDE NEURAL                            ║" << std::endl;
    std::cout << "║   Collision Avoidance System                       ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════╝" << std::endl;
    
    int passed = 0;
    int total = 6;
    
    if (test_network_creation()) passed++;
    if (test_forward_propagation()) passed++;
    if (test_training()) passed++;
    if (test_navigation_scenarios()) passed++;
    if (test_decision_consistency()) passed++;
    if (test_weight_saving()) passed++;
    
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "RESULTADO FINAL: " << passed << "/" << total << " testes passaram" << std::endl;
    std::cout << std::string(50, '=') << "\n" << std::endl;
    
    if (passed == total) {
        std::cout << "✓ TODOS OS TESTES PASSARAM!" << std::endl;
        std::cout << "  Sistema pronto para uso." << std::endl;
        return 0;
    } else {
        std::cout << "⚠ ALGUNS TESTES FALHARAM" << std::endl;
        std::cout << "  Revise o código antes de usar no robô." << std::endl;
        return 1;
    }
}
