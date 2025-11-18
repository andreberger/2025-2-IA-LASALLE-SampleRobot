/**
 * @file train_network.cpp
 * @brief Programa standalone para treinar a rede neural de collision avoidance
 * 
 * Este programa treina a rede neural e salva os pesos para uso posterior.
 * Útil para treinar offline sem necessidade do simulador.
 * 
 * Uso:
 *   ./build/train_network [output_weights_file]
 * 
 * Exemplo:
 *   ./build/train_network trained_weights.json
 */

#include "../include/neuralnetwork/NeuralNetwork.h"
#include "../include/neuralnetwork/ActivationFunction.h"
#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <string>

/**
 * @brief Cria o dataset de treinamento completo
 * 
 * Dataset baseado em análise de cenários de navegação:
 * - Entrada: [direita_livre, esquerda_livre, frente_livre, tras_livre]
 * - Saída: [ação_codificada]
 * 
 * Onde 1 = caminho livre, 0 = obstruído
 * E ação_codificada indica: direita(0.53), esquerda(0.59), frente(0.65), 
 *                           trás(0.71), parar(0.77)
 */
std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>>
createFullTrainingDataset() {
    
    std::cout << "\nCriando dataset de treinamento..." << std::endl;
    
    std::vector<std::vector<double>> inputs = {
        // ===== 1 direção livre (4 padrões) =====
        {1, 0, 0, 0},  // Apenas direita livre
        {0, 1, 0, 0},  // Apenas esquerda livre
        {0, 0, 1, 0},  // Apenas frente livre
        {0, 0, 0, 1},  // Apenas trás livre
        
        // ===== 2 direções livres (6 padrões) =====
        {0, 0, 1, 1},  // Frente e trás livres
        {1, 1, 0, 0},  // Direita e esquerda livres
        {0, 1, 1, 0},  // Esquerda e frente livres
        {1, 0, 0, 1},  // Direita e trás livres
        {1, 0, 1, 0},  // Direita e frente livres
        {0, 1, 0, 1},  // Esquerda e trás livres
        
        // ===== 3 direções livres (4 padrões) =====
        {0, 1, 1, 1},  // Tudo livre exceto direita
        {1, 0, 1, 1},  // Tudo livre exceto esquerda
        {1, 1, 0, 1},  // Tudo livre exceto frente
        {1, 1, 1, 0},  // Tudo livre exceto trás
        
        // ===== 4 direções livres (1 padrão) =====
        {1, 1, 1, 1},  // Tudo completamente livre
        
        // ===== Nenhuma direção livre (1 padrão) =====
        {0, 0, 0, 0}   // Tudo bloqueado - situação de emergência
    };
    
    // Targets correspondentes
    // Estratégia: priorizar movimento para frente quando possível
    std::vector<std::vector<double>> targets = {
        // 1 direção livre
        {0.53},  // DIREITA - única opção
        {0.59},  // ESQUERDA - única opção
        {0.65},  // FRENTE - única opção
        {0.71},  // TRÁS - única opção (última alternativa)
        
        // 2 direções livres
        {0.65},  // FRENTE preferido quando frente e trás livres
        {0.53},  // DIREITA preferida quando laterais livres
        {0.65},  // FRENTE preferido quando esquerda e frente livres
        {0.53},  // DIREITA preferida quando direita e trás livres
        {0.65},  // FRENTE preferido quando direita e frente livres
        {0.59},  // ESQUERDA preferida quando esquerda e trás livres
        
        // 3 direções livres
        {0.65},  // FRENTE - prioridade mesmo sem direita
        {0.65},  // FRENTE - prioridade mesmo sem esquerda
        {0.53},  // DIREITA - frente bloqueada, escolher lateral
        {0.65},  // FRENTE - melhor opção
        
        // 4 direções livres
        {0.65},  // FRENTE - seguir em frente quando tudo livre
        
        // Nenhuma direção livre
        {0.77}   // PARAR - sem opções seguras
    };
    
    std::cout << "  Total de padrões de treinamento: " << inputs.size() << std::endl;
    std::cout << "  Distribuição:" << std::endl;
    std::cout << "    - 1 direção livre:  4 padrões" << std::endl;
    std::cout << "    - 2 direções livres: 6 padrões" << std::endl;
    std::cout << "    - 3 direções livres: 4 padrões" << std::endl;
    std::cout << "    - 4 direções livres: 1 padrão" << std::endl;
    std::cout << "    - Tudo bloqueado:   1 padrão" << std::endl;
    
    return {inputs, targets};
}

/**
 * @brief Cria dataset de validação
 */
std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>>
createValidationDataset() {
    
    std::cout << "\nCriando dataset de validação..." << std::endl;
    
    std::vector<std::vector<double>> inputs = {
        {1, 0, 0, 0},  // Direita livre
        {0, 1, 0, 0},  // Esquerda livre
        {0, 0, 1, 0},  // Frente livre
        {0, 0, 0, 1},  // Trás livre
        {1, 1, 1, 1},  // Tudo livre
        {0, 0, 0, 0},  // Tudo bloqueado
        {1, 0, 1, 0},  // Direita e frente
        {0, 1, 1, 0}   // Esquerda e frente
    };
    
    std::vector<std::vector<double>> targets = {
        {0.53},  // DIREITA
        {0.59},  // ESQUERDA
        {0.65},  // FRENTE
        {0.71},  // TRÁS
        {0.65},  // FRENTE
        {0.77},  // PARAR
        {0.65},  // FRENTE
        {0.65}   // FRENTE
    };
    
    std::cout << "  Total de padrões de validação: " << inputs.size() << std::endl;
    
    return {inputs, targets};
}

/**
 * @brief Interpreta a saída da rede em ação legível
 */
std::string interpretOutput(double output) {
    if (output >= 0.50 && output < 0.56) return "DIREITA";
    if (output >= 0.56 && output < 0.62) return "ESQUERDA";
    if (output >= 0.62 && output < 0.68) return "FRENTE";
    if (output >= 0.68 && output < 0.74) return "TRÁS";
    if (output >= 0.74 && output < 0.80) return "PARAR";
    return "INDEFINIDO";
}

int main(int argc, char* argv[]) {
    std::cout << "\n╔════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║   TREINAMENTO DA REDE NEURAL                       ║" << std::endl;
    std::cout << "║   Collision Avoidance para Robôs Terrestres        ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════╝\n" << std::endl;
    
    // Nome do arquivo de saída
    std::string outputFile = "trained_weights.json";
    if (argc > 1) {
        outputFile = argv[1];
    }
    
    std::cout << "Arquivo de saída: " << outputFile << "\n" << std::endl;
    
    try {
        // Criar rede neural
        std::cout << "Criando arquitetura da rede neural..." << std::endl;
        NeuralNetwork network(4, 1, 0.3, 0.9);
        
        // Arquitetura: 4 -> 5 -> 1
        // - 4 entradas: [direita, esquerda, frente, trás]
        // - 5 neurônios na camada oculta (ativação sigmoid)
        // - 1 saída: ação codificada (ativação sigmoid)
        network.addHiddenLayer(5, std::make_shared<SigmoidActivation>(), 0.5);
        network.finalize(std::make_shared<SigmoidActivation>(), 0.5);
        
        std::cout << network.getArchitectureInfo() << "\n" << std::endl;
        
        // Obter datasets
        std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>> trainingData = createFullTrainingDataset();
        std::vector<std::vector<double>>& trainingInputs = trainingData.first;
        std::vector<std::vector<double>>& trainingTargets = trainingData.second;
        
        std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>> validationData = createValidationDataset();
        std::vector<std::vector<double>>& validationInputs = validationData.first;
        std::vector<std::vector<double>>& validationTargets = validationData.second;
        
        // Treinar a rede
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "INICIANDO TREINAMENTO" << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        
        int epochs = network.trainBatch(
            trainingInputs, 
            trainingTargets,
            100000,      // Máximo de épocas
            0.004,       // Threshold de erro
            true         // Verbose
        );
        
        // Validar a rede
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "VALIDAÇÃO" << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        
        double validationError = network.validate(validationInputs, validationTargets, true);
        
        // Teste interativo com alguns cenários
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "TESTES DE CENÁRIOS" << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        
        std::vector<std::pair<std::vector<double>, std::string>> testCases = {
            {{0, 0, 1, 0}, "Corredor: apenas frente livre"},
            {{1, 1, 0, 0}, "Cruzamento: direita e esquerda livres"},
            {{1, 0, 1, 0}, "Canto: direita e frente livres"},
            {{0, 1, 1, 0}, "Canto: esquerda e frente livres"},
            {{1, 1, 1, 1}, "Espaço aberto: tudo livre"},
            {{0, 0, 0, 0}, "Bloqueio total: sem saída"}
        };
        
        for (size_t tc = 0; tc < testCases.size(); ++tc) {
            const std::vector<double>& input = testCases[tc].first;
            const std::string& description = testCases[tc].second;
            
            std::vector<double> output = network.predict(input);
            std::cout << "\n" << description << std::endl;
            std::cout << "  Input: [";
            for (size_t i = 0; i < input.size(); ++i) {
                std::cout << (input[i] > 0.5 ? "Livre" : "Bloq");
                if (i < input.size() - 1) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
            std::cout << "  Saída da rede: " << std::fixed << std::setprecision(4) 
                     << output[0] << std::endl;
            std::cout << "  Ação decidida: " << interpretOutput(output[0]) << std::endl;
        }
        
        // Salvar pesos
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "SALVANDO MODELO" << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        
        if (network.saveWeights(outputFile)) {
            std::cout << "\n✓ Modelo treinado salvo com sucesso!" << std::endl;
            std::cout << "  Arquivo: " << outputFile << std::endl;
            std::cout << "  Épocas de treinamento: " << epochs << std::endl;
            std::cout << "  Erro de validação: " << std::fixed << std::setprecision(6) 
                     << validationError << std::endl;
        } else {
            std::cerr << "\n✗ Erro ao salvar modelo" << std::endl;
            return 1;
        }
        
        std::cout << "\n╔════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║   TREINAMENTO CONCLUÍDO COM SUCESSO!               ║" << std::endl;
        std::cout << "╚════════════════════════════════════════════════════╝\n" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\n✗ ERRO: " << e.what() << std::endl;
        return 1;
    }
}
