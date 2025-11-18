#ifndef NEURALCOLLISIONAVOIDANCE_H
#define NEURALCOLLISIONAVOIDANCE_H

#include "Aria.h"
#include "ClassRobo.h"
#include "neuralnetwork/NeuralNetwork.h"
#include <memory>
#include <string>

/**
 * @brief Sistema de collision avoidance baseado em rede neural
 * 
 * Esta classe substitui a abordagem heurística tradicional por uma
 * rede neural treinada para tomar decisões de navegação baseadas
 * nas leituras dos sensores do robô.
 * 
 * Arquitetura:
 * - Input: 4 valores normalizados dos sensores (direita, esquerda, frente, trás)
 * - Hidden layers: camadas ocultas configuráveis
 * - Output: 1 valor que mapeia para uma ação de movimento
 * 
 * Mapeamento de ações (baseado na saída da rede):
 * - 0.50 - 0.56: Virar à DIREITA
 * - 0.56 - 0.62: Virar à ESQUERDA  
 * - 0.62 - 0.68: Seguir em FRENTE
 * - 0.68 - 0.74: Mover para TRÁS
 * - 0.74 - 0.80: PARAR
 */
class NeuralCollisionAvoidance : public ArASyncTask {
private:
    PioneerRobot* robo;
    std::unique_ptr<NeuralNetwork> network;
    
    ArCondition myCondition;
    ArMutex myMutex;
    
    // Dados dos sensores
    int sonar[8];
    
    // Thresholds para normalização dos sensores
    static constexpr double NEAR_THRESHOLD = 20.0;  // Obstáculo próximo
    static constexpr double FAR_THRESHOLD = 100.0;  // Obstáculo distante
    
    // Intervalos de ação baseados na saída da rede
    static constexpr double ACTION_RIGHT_MIN = 0.50;
    static constexpr double ACTION_RIGHT_MAX = 0.56;
    static constexpr double ACTION_LEFT_MIN = 0.56;
    static constexpr double ACTION_LEFT_MAX = 0.62;
    static constexpr double ACTION_FORWARD_MIN = 0.62;
    static constexpr double ACTION_FORWARD_MAX = 0.68;
    static constexpr double ACTION_BACKWARD_MIN = 0.68;
    static constexpr double ACTION_BACKWARD_MAX = 0.74;
    static constexpr double ACTION_STOP_MIN = 0.74;
    static constexpr double ACTION_STOP_MAX = 0.80;
    
    // Velocidades de movimento
    static constexpr int VELOCITY_MOVE = 200;
    static constexpr int VELOCITY_ROTATION = 50;
    static constexpr int ROTATION_ANGLE = 30;
    
    // Estatísticas e logging
    int decisionCount;
    int rightDecisions;
    int leftDecisions;
    int forwardDecisions;
    int backwardDecisions;
    int stopDecisions;

public:
    /**
     * @brief Construtor
     * @param _robo Ponteiro para o objeto do robô Pioneer
     */
    explicit NeuralCollisionAvoidance(PioneerRobot* _robo);
    
    /**
     * @brief Inicializa e treina a rede neural
     * @param weightsFile Arquivo de pesos pré-treinados (opcional)
     * @return true se inicialização bem-sucedida
     */
    bool initializeNetwork(const std::string& weightsFile = "");
    
    /**
     * @brief Thread principal de execução
     */
    void* runThread(void*) override;
    
    /**
     * @brief Aguarda condição
     */
    void waitOnCondition();
    
    /**
     * @brief Trava o mutex
     */
    void lockMutex();
    
    /**
     * @brief Destrava o mutex
     */
    void unlockMutex();
    
    /**
     * @brief Salva os pesos da rede neural treinada
     * @param filename Nome do arquivo
     * @return true se salvou com sucesso
     */
    bool saveNetworkWeights(const std::string& filename);
    
    /**
     * @brief Exibe estatísticas de decisões tomadas
     */
    void printStatistics() const;

private:
    /**
     * @brief Normaliza os valores dos sensores para input da rede
     * @param sensorValues Array com 8 valores de sensores sonar
     * @return Vetor normalizado [direita, esquerda, frente, trás]
     */
    std::vector<double> normalizeSensorData(const int* sensorValues);
    
    /**
     * @brief Interpreta a saída da rede e executa a ação correspondente
     * @param networkOutput Saída da rede neural
     */
    void executeAction(double networkOutput);
    
    /**
     * @brief Cria o dataset de treinamento
     * @return Par de vetores: inputs e targets
     */
    std::pair<std::vector<std::vector<double>>, 
              std::vector<std::vector<double>>> createTrainingData();
    
    /**
     * @brief Cria dataset de validação
     * @return Par de vetores: inputs e targets
     */
    std::pair<std::vector<std::vector<double>>, 
              std::vector<std::vector<double>>> createValidationData();
    
    /**
     * @brief Determina a direção com mais espaço livre
     * @return 1 para esquerda, 2 para direita
     */
    int determineBestDirection() const;
};

#endif // NEURALCOLLISIONAVOIDANCE_H
