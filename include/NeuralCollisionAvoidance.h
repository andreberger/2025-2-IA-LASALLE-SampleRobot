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
    
    // Dados dos sensores sonar do robô Pioneer
    // O Pioneer possui 8 sensores sonar distribuídos ao redor do chassis
    // Índices: 0=direita, 1-2=diagonal direita, 3-4=frente, 5-6=diagonal esquerda, 7=esquerda
    int sonar[8];
    
    // ===== SISTEMA DE 3 ZONAS DE SEGURANÇA =====
    // Este foi o grande diferencial implementado para evitar colisões
    // Inspirado em sistemas de freio automático de carros modernos
    
    // Thresholds para normalização dos sensores (valores em mm)
    // PROBLEMA ORIGINAL: valores muito altos causavam paradas constantes
    // SOLUÇÃO: ajuste fino através de testes iterativos no simulador
    
    static constexpr double NEAR_THRESHOLD = 600.0;    
    // ZONA DE ALERTA (< 600mm)
    // Quando sensor detecta obstáculo abaixo de 600mm, considera "bloqueado"
    // Ativa sistema de DESVIO INTELIGENTE
    // Não para o robô, mas força uma curva para o lado livre
    
    static constexpr double DANGER_THRESHOLD = 250.0;  
    // ZONA DE PERIGO (< 250mm)
    // Obstáculo MUITO próximo - risco iminente de colisão
    // Ativa PARADA DE EMERGÊNCIA imediata
    // Ignora todas as outras decisões - prioridade máxima
    
    static constexpr double FAR_THRESHOLD = 1500.0;    
    // ZONA LIVRE (> 1500mm)
    // Obstáculo distante - caminho considerado totalmente livre
    // Usado para otimizações futuras (não implementado nesta versão)
    
    // ===== CODIFICAÇÃO DAS AÇÕES DA REDE NEURAL =====
    // A rede neural tem apenas 1 saída (valor entre 0 e 1)
    // Dividimos esse intervalo em 5 faixas, cada uma representando uma ação
    // Isso é chamado de "codificação por intervalos" ou "discretização"
    
    // Intervalos de ação baseados na saída da rede
    // Esses valores foram escolhidos para maximizar a separação entre ações
    // e minimizar ambiguidades na decisão
    
    static constexpr double ACTION_RIGHT_MIN = 0.50;   // Início da faixa "virar direita"
    static constexpr double ACTION_RIGHT_MAX = 0.56;   // Fim da faixa "virar direita"
    static constexpr double ACTION_LEFT_MIN = 0.56;    // Início da faixa "virar esquerda"
    static constexpr double ACTION_LEFT_MAX = 0.62;    // Fim da faixa "virar esquerda"
    static constexpr double ACTION_FORWARD_MIN = 0.62; // Início da faixa "seguir em frente"
    static constexpr double ACTION_FORWARD_MAX = 0.68; // Fim da faixa "seguir em frente"
    static constexpr double ACTION_BACKWARD_MIN = 0.68;// Início da faixa "andar para trás"
    static constexpr double ACTION_BACKWARD_MAX = 0.74;// Fim da faixa "andar para trás"
    static constexpr double ACTION_STOP_MIN = 0.74;    // Início da faixa "parar"
    static constexpr double ACTION_STOP_MAX = 0.80;    // Fim da faixa "parar"
    
    // ===== PARÂMETROS DE MOVIMENTO =====
    // Ajustados após testes para balancear velocidade e segurança
    
    // Velocidades de movimento (em unidades do Pioneer)
    static constexpr int VELOCITY_MOVE = 150;         
    // Velocidade linear reduzida (era 200, causava reação lenta)
    // 150 dá tempo suficiente para o robô reagir aos obstáculos
    
    static constexpr int VELOCITY_ROTATION = 40;      
    // Velocidade de rotação (girar no próprio eixo)
    // Valor baixo para curvas suaves e controladas
    
    static constexpr int ROTATION_ANGLE = 45;          
    // Ângulo de rotação aumentado de 30° para 45°
    // 30° era insuficiente para desviar efetivamente
    // 45° permite contornar obstáculos com mais facilidade
    
    // ===== ESTATÍSTICAS E MONITORAMENTO =====
    // Para análise de comportamento e apresentação dos resultados
    // Permitem verificar se o robô está tomando decisões balanceadas
    int decisionCount;
    int rightDecisions;
    int leftDecisions;
    int forwardDecisions;
    int backwardDecisions;
    int stopDecisions;

public:
    /**
     * @brief Construtor da classe
     * @param _robo Ponteiro para o objeto do robô Pioneer
     * 
     * Inicializa o sistema de collision avoidance neural
     * Prepara estruturas de dados e zera estatísticas
     */
    explicit NeuralCollisionAvoidance(PioneerRobot* _robo);
    
    /**
     * @brief Inicializa e treina a rede neural
     * @param weightsFile Arquivo de pesos pré-treinados (opcional)
     * @return true se inicialização bem-sucedida
     * 
     * DUAS FORMAS DE USAR:
     * 1. COM arquivo de pesos: carrega rede já treinada (MAIS RÁPIDO)
     * 2. SEM arquivo: treina do zero (útil para experimentos)
     * 
     * Para o trabalho, usamos pesos pré-treinados salvos em trained_weights.json
     * Isso garante comportamento consistente entre execuções
     */
    bool initializeNetwork(const std::string& weightsFile = "");
    
    /**
     * @brief Thread principal de execução do sistema
     * 
     * CICLO DE VIDA:
     * 1. Lê sensores sonar (8 valores)
     * 2. Normaliza dados (agrupa em 4 direções)
     * 3. Passa pela rede neural (decisão)
     * 4. Executa ação no robô
     * 5. Loop a cada 100ms
     * 
     * Roda em thread separada para não bloquear outras operações
     */
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
