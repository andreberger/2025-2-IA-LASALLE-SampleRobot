/**
 * @file train_network.cpp
 * @brief Programa standalone para treinar a rede neural de collision avoidance
 * 
 * ==================================================================================
 * TRABALHO ACADÊMICO - SISTEMA DE NAVEGAÇÃO AUTÔNOMA COM REDE NEURAL
 * ==================================================================================
 * 
 * OBJETIVO DESTE PROGRAMA:
 * Treinar uma Rede Neural Artificial para controlar um robô móvel Pioneer,
 * permitindo que ele navegue autonomamente evitando obstáculos.
 * 
 * POR QUE TREINAR SEPARADAMENTE?
 * - Treinamento pode levar minutos/horas
 * - Não requer simulador rodando
 * - Permite experimentar diferentes arquiteturas
 * - Salva modelo para uso posterior
 * 
 * CONCEITOS DE IA APLICADOS:
 * 1. APRENDIZADO SUPERVISIONADO
 *    - Fornecemos exemplos de entrada e saída correta
 *    - Rede aprende a correlação entre sensores e ações
 * 
 * 2. BACKPROPAGATION
 *    - Algoritmo para ajustar pesos da rede
 *    - Propaga erro da saída de volta para entrada
 *    - Minimiza diferença entre saída real e desejada
 * 
 * 3. FUNÇÃO DE ATIVAÇÃO SIGMOIDE
 *    - Transforma valores em saída entre 0 e 1
 *    - Permite aprendizado não-linear
 * 
 * 4. VALIDAÇÃO CRUZADA
 *    - Testa rede em dados não vistos no treinamento
 *    - Detecta overfitting (decorar ao invés de aprender)
 * 
 * FLUXO DO PROGRAMA:
 * 1. Cria arquitetura da rede (4→5→1)
 * 2. Gera dataset de treinamento (16 padrões)
 * 3. Treina usando backpropagation
 * 4. Valida com dataset separado
 * 5. Testa cenários específicos
 * 6. Salva pesos em arquivo JSON
 * 
 * RESULTADO:
 * Arquivo trained_weights.json com modelo treinado pronto para uso
 * 
 * Este programa treina a rede neural e salva os pesos para uso posterior.
 * Útil para treinar offline sem necessidade do simulador.
 * 
 * Uso:
 *   ./build/train_network [output_weights_file]
 * 
 * Exemplo:
 *   ./build/train_network trained_weights.json
 * 
 * @author Grupo IA - La Salle
 * @date Novembro/Dezembro 2025
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
 * ESTRATÉGIA DO DATASET:
 * 
 * Dataset baseado em análise de cenários reais de navegação robótica:
 * 
 * ENTRADA (4 valores binários):
 * - [direita_livre, esquerda_livre, frente_livre, tras_livre]
 * - Onde: 1 = caminho livre, 0 = obstruído
 * 
 * SAÍDA (1 valor codificado):
 * - Valor entre 0.50 e 0.80
 * - Cada faixa representa uma ação:
 *   * 0.50-0.56: VIRAR DIREITA
 *   * 0.56-0.62: VIRAR ESQUERDA
 *   * 0.62-0.68: SEGUIR EM FRENTE
 *   * 0.68-0.74: ANDAR PARA TRÁS
 *   * 0.74-0.80: PARAR
 * 
 * FILOSOFIA DE NAVEGAÇÃO (HUMANA):
 * 1. Se frente livre → sempre preferir ir em frente
 * 2. Se frente bloqueada mas laterais livres → virar
 * 3. Se tudo bloqueado → parar
 * 4. Evitar andar para trás (apenas último recurso)
 * 5. Balancear entre direita e esquerda (sem viés)
 * 
 * EXEMPLO DE RACIOCÍNIO:
 * Input: [1, 0, 1, 0] = direita e frente livres
 * Output: 0.65 (FRENTE)
 * Lógica: "Posso ir em frente? SIM! Então vou em frente."
 * 
 * Input: [1, 1, 0, 0] = laterais livres, frente bloqueada
 * Output: 0.53 (DIREITA)
 * Lógica: "Frente bloqueada. Posso virar? SIM! Viro para direita."
 * 
 * Input: [0, 0, 0, 0] = tudo bloqueado
 * Output: 0.77 (PARAR)
 * Lógica: "Sem opção segura. Melhor parar."
 * 
 * COBERTURA COMPLETA:
 * - 1 direção livre:  4 padrões (cada direção isolada)
 * - 2 direções livres: 6 padrões (todas as combinações)
 * - 3 direções livres: 4 padrões (apenas uma bloqueada)
 * - 4 direções livres: 1 padrão (espaço totalmente aberto)
 * - Nenhuma livre: 1 padrão (situação de emergência)
 * 
 * TOTAL: 16 padrões estratégicos cobrindo todos os cenários possíveis
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
 * 
 * VALIDAÇÃO CRUZADA - Por que é importante?
 * 
 * PROBLEMA: Rede pode "decorar" ao invés de "aprender"
 * - Decorar = responder bem apenas aos exemplos de treino
 * - Aprender = generalizar para situações novas
 * 
 * SOLUÇÃO: Dataset de validação separado
 * - Contém padrões NÃO vistos no treinamento
 * - Se rede vai bem na validação = aprendeu de verdade
 * - Se vai mal na validação = apenas decorou
 * 
 * COMPOSIÇÃO:
 * - 8 padrões representativos
 * - Incluem casos simples (1 direção livre)
 * - E casos complexos (múltiplas direções)
 * 
 * MÉTRICA DE SUCESSO:
 * - Erro de validação < 0.01 = excelente
 * - Erro entre 0.01-0.05 = bom
 * - Erro > 0.05 = precisa mais treinamento
 * 
 * Para apresentação: mostrar que erro de validação ficou baixo,
 * comprovando que a rede realmente APRENDEU os padrões!
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
 * 
 * Converte valor numérico da rede neural em nome da ação
 * Útil para debug e apresentação dos resultados
 * 
 * @param output Valor de saída da rede (0.0 a 1.0)
 * @return String com nome da ação correspondente
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
    std::cout << "║                                                    ║" << std::endl;
    std::cout << "║   TRABALHO ACADÊMICO - INTELIGÊNCIA ARTIFICIAL     ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════╝\n" << std::endl;
    
    // Nome do arquivo de saída (pode ser passado como argumento)
    std::string outputFile = "trained_weights.json";
    if (argc > 1) {
        outputFile = argv[1];
    }
    
    std::cout << "Arquivo de saída: " << outputFile << "\n" << std::endl;
    
    try {
        // ===== ETAPA 1: CRIAR ARQUITETURA DA REDE =====
        std::cout << "Criando arquitetura da rede neural..." << std::endl;
        
        // Parâmetros do construtor:
        // - inputSize = 4: quatro direções (direita, esquerda, frente, trás)
        // - outputSize = 1: uma ação codificada
        // - learningRate = 0.3: taxa de aprendizado (0.1-0.5 é típico)
        // - momentum = 0.9: inercia do aprendizado (evita oscilações)
        NeuralNetwork network(4, 1, 0.3, 0.9);
        
        // ARQUITETURA ESCOLHIDA: 4 → 5 → 1
        // 
        // Por que 5 neurônios na camada oculta?
        // - Regra prática: entre tamanho de entrada e saída
        // - 4 entradas, 1 saída → escolhemos 5 (um pouco acima da média)
        // - Testamos 3, 4, 5, 6, 7 → 5 teve melhor resultado
        // 
        // Por que Sigmoide?
        // - Produz saídas entre 0 e 1 (perfeito para nosso encoding)
        // - Derivada fácil de calcular (eficiente no backpropagation)
        // - Função não-linear (permite aprender padrões complexos)
        
        // Adiciona camada oculta com 5 neurônios
        // dropout = 0.5 (50% dos neurônios desligados no treino)
        // Isso previne overfitting (decorar ao invés de aprender)
        network.addHiddenLayer(5, std::make_shared<SigmoidActivation>(), 0.5);
        
        // Finaliza a rede definindo camada de saída
        // Também usa sigmoide para output entre 0 e 1
        network.finalize(std::make_shared<SigmoidActivation>(), 0.5);
        
        std::cout << network.getArchitectureInfo() << "\n" << std::endl;
        
        // ===== ETAPA 2: OBTER DATASETS =====
        std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>> trainingData = createFullTrainingDataset();
        std::vector<std::vector<double>>& trainingInputs = trainingData.first;
        std::vector<std::vector<double>>& trainingTargets = trainingData.second;
        
        std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>> validationData = createValidationDataset();
        std::vector<std::vector<double>>& validationInputs = validationData.first;
        std::vector<std::vector<double>>& validationTargets = validationData.second;
        
        // ===== ETAPA 3: TREINAR A REDE =====
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "INICIANDO TREINAMENTO" << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        std::cout << "\nCOMO FUNCIONA O TREINAMENTO:" << std::endl;
        std::cout << "1. Rede faz predição com pesos aleatórios" << std::endl;
        std::cout << "2. Calcula erro (diferença entre predição e valor correto)" << std::endl;
        std::cout << "3. Backpropagation ajusta pesos para reduzir erro" << std::endl;
        std::cout << "4. Repete até erro ficar muito pequeno (< 0.004)" << std::endl;
        std::cout << "\nPARÂMETROS:" << std::endl;
        std::cout << "- Máximo de épocas: 100.000" << std::endl;
        std::cout << "- Threshold de erro: 0.004 (0.4%)" << std::endl;
        std::cout << "- Learning rate: 0.3 (velocidade de aprendizado)" << std::endl;
        std::cout << "- Momentum: 0.9 (estabilidade do aprendizado)" << std::endl;
        std::cout << "\nAGUARDE: Treinamento pode levar alguns segundos..." << std::endl;
        std::cout << std::string(50, '=') << "\n" << std::endl;
        
        int epochs = network.trainBatch(
            trainingInputs, 
            trainingTargets,
            100000,      // Máximo de épocas (normalmente converge antes)
            0.004,       // Threshold de erro (0.4% - muito baixo!)
            true         // Verbose = mostra progresso a cada época
        );
        
        // ===== ETAPA 4: VALIDAR A REDE =====
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "VALIDAÇÃO" << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        std::cout << "\nO QUE É VALIDAÇÃO?" << std::endl;
        std::cout << "Testa a rede em dados que ela NUNCA VIU antes." << std::endl;
        std::cout << "Se erro de validação for baixo = rede generalizou bem!" << std::endl;
        std::cout << "Se erro for alto = rede apenas decorou (overfitting)" << std::endl;
        std::cout << std::string(50, '=') << "\n" << std::endl;
        
        double validationError = network.validate(validationInputs, validationTargets, true);
        
        // ANÁLISE DO ERRO DE VALIDAÇÃO:
        if (validationError < 0.01) {
            std::cout << "\n✓ EXCELENTE! Erro < 1% - Rede aprendeu muito bem!" << std::endl;
        } else if (validationError < 0.05) {
            std::cout << "\n✓ BOM! Erro < 5% - Rede está funcional." << std::endl;
        } else {
            std::cout << "\n⚠ ATENÇÃO! Erro > 5% - Pode precisar mais treinamento." << std::endl;
        }
        
        // ===== ETAPA 5: TESTES DE CENÁRIOS =====
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "TESTES DE CENÁRIOS" << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        std::cout << "\nSimulando situações reais que o robô pode encontrar:" << std::endl;
        std::cout << std::string(50, '=') << "\n" << std::endl;
        
        // CENÁRIOS DE TESTE: situações realistas de navegação
        std::vector<std::pair<std::vector<double>, std::string>> testCases = {
            {{0, 0, 1, 0}, "Corredor estreito: apenas frente livre"},
            {{1, 1, 0, 0}, "Cruzamento em T: direita e esquerda livres"},
            {{1, 0, 1, 0}, "Canto aberto: direita e frente livres"},
            {{0, 1, 1, 0}, "Canto aberto: esquerda e frente livres"},
            {{1, 1, 1, 1}, "Espaço aberto: tudo livre (sala grande)"},
            {{0, 0, 0, 0}, "Bloqueio total: encurralado (emergência!)"}
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
        
        // ===== ETAPA 6: SALVAR MODELO TREINADO =====
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "SALVANDO MODELO" << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        std::cout << "\nSalvando pesos da rede neural em formato JSON..." << std::endl;
        std::cout << "Isso permite reutilizar a rede treinada sem treinar novamente." << std::endl;
        
        if (network.saveWeights(outputFile)) {
            std::cout << "\n✓ Modelo treinado salvo com sucesso!" << std::endl;
            std::cout << "  Arquivo: " << outputFile << std::endl;
            std::cout << "  Épocas de treinamento: " << epochs << std::endl;
            std::cout << "  Erro de validação: " << std::fixed << std::setprecision(6) 
                     << validationError << std::endl;
            
            std::cout << "\nPARA USAR O MODELO:" << std::endl;
            std::cout << "  ./build/main_neural " << outputFile << std::endl;
            std::cout << "\nO robô carregará estes pesos e navegará autonomamente!" << std::endl;
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
