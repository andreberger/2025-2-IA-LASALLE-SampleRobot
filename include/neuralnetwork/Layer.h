#ifndef LAYER_H
#define LAYER_H

#include <vector>
#include <memory>
#include <random>
#include "ActivationFunction.h"

/**
 * @brief Representa uma camada (layer) na rede neural
 * 
 * Cada camada contém neurônios com pesos, bias e função de ativação.
 * Implementa forward propagation e backward propagation para treinamento.
 * 
 * Estrutura:
 * - neurons: número de neurônios nesta camada
 * - inputSize: número de entradas que cada neurônio recebe
 * - weights: matriz [inputSize][neurons] de pesos sinápticos
 * - bias: vetor [neurons] de bias para cada neurônio
 * - activations: saídas dos neurônios após ativação
 * - inputs: entradas recebidas (guardadas para backpropagation)
 */
class Layer {
private:
    int inputSize;      // Número de entradas que esta camada recebe
    int neurons;        // Número de neurônios nesta camada
    
    // Pesos: weights[i][j] = peso da entrada i para o neurônio j
    std::vector<std::vector<double>> weights;
    
    // Bias: bias[j] = bias do neurônio j
    std::vector<double> bias;
    
    // Função de ativação desta camada
    std::shared_ptr<ActivationFunction> activationFunction;
    
    // Valores intermediários (guardados para backpropagation)
    std::vector<double> inputs;          // Entradas recebidas
    std::vector<double> weightedSums;    // Somas ponderadas (antes da ativação)
    std::vector<double> activations;     // Saídas após ativação
    
    // Gradientes (para backpropagation)
    std::vector<std::vector<double>> weightGradients;
    std::vector<double> biasGradients;
    
    // Momentum (para otimização)
    std::vector<std::vector<double>> weightVelocity;
    std::vector<double> biasVelocity;

public:
    /**
     * @brief Construtor da camada
     * @param inputSize Número de entradas
     * @param neurons Número de neurônios nesta camada
     * @param activationFunc Função de ativação a ser utilizada
     * @param weightInitRange Amplitude para inicialização aleatória dos pesos [-range, +range]
     */
    Layer(int inputSize, int neurons, 
          std::shared_ptr<ActivationFunction> activationFunc,
          double weightInitRange = 0.5);
    
    /**
     * @brief Forward propagation - calcula as saídas da camada
     * @param input Vetor de entradas
     * @return Vetor de saídas (ativações)
     */
    std::vector<double> forward(const std::vector<double>& input);
    
    /**
     * @brief Backward propagation - calcula gradientes
     * @param outputGradients Gradientes vindos da camada seguinte
     * @return Gradientes a serem propagados para a camada anterior
     */
    std::vector<double> backward(const std::vector<double>& outputGradients);
    
    /**
     * @brief Atualiza os pesos e bias usando os gradientes calculados
     * @param learningRate Taxa de aprendizado
     * @param momentum Fator de momentum para suavizar atualizações
     */
    void updateWeights(double learningRate, double momentum);
    
    /**
     * @brief Retorna o número de neurônios desta camada
     * @return Número de neurônios
     */
    int getOutputSize() const { return neurons; }
    
    /**
     * @brief Retorna o número de entradas desta camada
     * @return Número de entradas
     */
    int getInputSize() const { return inputSize; }
    
    /**
     * @brief Retorna as últimas ativações calculadas
     * @return Vetor de ativações
     */
    const std::vector<double>& getActivations() const { return activations; }
    
    /**
     * @brief Define os pesos da camada (útil para carregar modelo salvo)
     * @param newWeights Matriz de pesos
     */
    void setWeights(const std::vector<std::vector<double>>& newWeights);
    
    /**
     * @brief Define os bias da camada (útil para carregar modelo salvo)
     * @param newBias Vetor de bias
     */
    void setBias(const std::vector<double>& newBias);
    
    /**
     * @brief Obtém os pesos atuais da camada
     * @return Matriz de pesos
     */
    const std::vector<std::vector<double>>& getWeights() const { return weights; }
    
    /**
     * @brief Obtém os bias atuais da camada
     * @return Vetor de bias
     */
    const std::vector<double>& getBias() const { return bias; }
    
    /**
     * @brief Obtém o nome da função de ativação
     * @return Nome da função de ativação
     */
    std::string getActivationName() const { 
        return activationFunction->getName(); 
    }

private:
    /**
     * @brief Inicializa os pesos aleatoriamente
     * @param range Amplitude da inicialização
     */
    void initializeWeights(double range);
};

#endif // LAYER_H
