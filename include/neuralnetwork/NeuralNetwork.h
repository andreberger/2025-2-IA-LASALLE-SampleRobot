#ifndef NEURALNETWORK_H
#define NEURALNETWORK_H

#include <vector>
#include <memory>
#include <string>
#include "Layer.h"
#include "ActivationFunction.h"

/**
 * @brief Classe principal da rede neural feedforward
 * 
 * Esta classe gerencia uma rede neural multicamadas completa,
 * incluindo treinamento, predição e persistência de modelo.
 * 
 * Arquitetura suportada:
 * - Rede feedforward totalmente conectada
 * - Número arbitrário de camadas ocultas
 * - Funções de ativação configuráveis por camada
 * - Treinamento por backpropagation com momentum
 * 
 * Exemplo de uso para collision avoidance:
 * - Input: [sensor_direita, sensor_esquerda, sensor_frente, sensor_tras]
 * - Hidden: camadas ocultas com funções de ativação não-lineares
 * - Output: [ação] onde ação indica direção (direita, esquerda, frente, etc.)
 */
class NeuralNetwork {
private:
    std::vector<std::shared_ptr<Layer>> layers;  // Camadas da rede
    int inputSize;                               // Dimensão da entrada
    int outputSize;                              // Dimensão da saída
    
    // Hiperparâmetros de treinamento
    double learningRate;
    double momentum;
    
    // Métricas de treinamento
    double lastError;
    int trainingIterations;

public:
    /**
     * @brief Construtor da rede neural
     * @param inputSize Número de entradas da rede
     * @param outputSize Número de saídas da rede
     * @param learningRate Taxa de aprendizado inicial
     * @param momentum Fator de momentum para otimização
     */
    NeuralNetwork(int inputSize, int outputSize, 
                  double learningRate = 0.3, 
                  double momentum = 0.9);
    
    /**
     * @brief Adiciona uma camada oculta à rede
     * @param neurons Número de neurônios na camada
     * @param activationFunc Função de ativação
     * @param weightInitRange Amplitude de inicialização dos pesos
     * 
     * Nota: As camadas devem ser adicionadas em ordem, da entrada para a saída
     */
    void addHiddenLayer(int neurons, 
                       std::shared_ptr<ActivationFunction> activationFunc,
                       double weightInitRange = 0.5);
    
    /**
     * @brief Finaliza a construção da rede, adicionando a camada de saída
     * @param activationFunc Função de ativação da camada de saída
     * @param weightInitRange Amplitude de inicialização dos pesos
     */
    void finalize(std::shared_ptr<ActivationFunction> activationFunc,
                 double weightInitRange = 0.5);
    
    /**
     * @brief Realiza predição (forward propagation) com entrada fornecida
     * @param input Vetor de entrada
     * @return Vetor de saída da rede
     */
    std::vector<double> predict(const std::vector<double>& input);
    
    /**
     * @brief Treina a rede com um único exemplo
     * @param input Vetor de entrada
     * @param target Vetor de saída esperada
     * @return Erro quadrático médio para este exemplo
     */
    double train(const std::vector<double>& input, 
                const std::vector<double>& target);
    
    /**
     * @brief Treina a rede com um conjunto de dados
     * @param inputs Conjunto de vetores de entrada
     * @param targets Conjunto de vetores de saída esperada
     * @param epochs Número de épocas de treinamento
     * @param errorThreshold Limiar de erro para parada antecipada
     * @param verbose Se true, exibe progresso do treinamento
     * @return Número de épocas executadas
     */
    int trainBatch(const std::vector<std::vector<double>>& inputs,
                  const std::vector<std::vector<double>>& targets,
                  int epochs,
                  double errorThreshold = 0.001,
                  bool verbose = true);
    
    /**
     * @brief Valida a rede com conjunto de dados de validação
     * @param inputs Conjunto de vetores de entrada
     * @param targets Conjunto de vetores de saída esperada
     * @param verbose Se true, exibe resultados detalhados
     * @return Erro médio no conjunto de validação
     */
    double validate(const std::vector<std::vector<double>>& inputs,
                   const std::vector<std::vector<double>>& targets,
                   bool verbose = true);
    
    /**
     * @brief Salva os pesos da rede em arquivo JSON
     * @param filename Nome do arquivo
     * @return true se salvou com sucesso
     */
    bool saveWeights(const std::string& filename) const;
    
    /**
     * @brief Carrega os pesos da rede de arquivo JSON
     * @param filename Nome do arquivo
     * @return true se carregou com sucesso
     */
    bool loadWeights(const std::string& filename);
    
    /**
     * @brief Define a taxa de aprendizado
     * @param lr Nova taxa de aprendizado
     */
    void setLearningRate(double lr) { learningRate = lr; }
    
    /**
     * @brief Define o fator de momentum
     * @param m Novo fator de momentum
     */
    void setMomentum(double m) { momentum = m; }
    
    /**
     * @brief Obtém o último erro calculado
     * @return Último erro
     */
    double getLastError() const { return lastError; }
    
    /**
     * @brief Obtém o número de iterações de treinamento realizadas
     * @return Número de iterações
     */
    int getTrainingIterations() const { return trainingIterations; }
    
    /**
     * @brief Obtém informações sobre a arquitetura da rede
     * @return String descrevendo a arquitetura
     */
    std::string getArchitectureInfo() const;

private:
    /**
     * @brief Calcula o erro quadrático médio
     * @param output Saída da rede
     * @param target Saída esperada
     * @return Erro quadrático médio
     */
    double calculateError(const std::vector<double>& output,
                         const std::vector<double>& target) const;
    
    /**
     * @brief Calcula os gradientes da camada de saída
     * @param output Saída da rede
     * @param target Saída esperada
     * @return Gradientes para backpropagation
     */
    std::vector<double> calculateOutputGradients(
        const std::vector<double>& output,
        const std::vector<double>& target) const;
};

#endif // NEURALNETWORK_H
