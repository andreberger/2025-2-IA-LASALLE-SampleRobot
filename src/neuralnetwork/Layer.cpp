#include "../include/neuralnetwork/Layer.h"
#include <random>
#include <ctime>
#include <iostream>
#include <stdexcept>

Layer::Layer(int inputSize, int neurons, 
             std::shared_ptr<ActivationFunction> activationFunc,
             double weightInitRange)
    : inputSize(inputSize),
      neurons(neurons),
      activationFunction(activationFunc) {
    
    if (inputSize <= 0 || neurons <= 0) {
        throw std::invalid_argument("Input size and neurons must be positive");
    }
    
    // Inicializar estruturas de dados
    weights.resize(inputSize, std::vector<double>(neurons));
    bias.resize(neurons, 0.0);
    
    weightGradients.resize(inputSize, std::vector<double>(neurons, 0.0));
    biasGradients.resize(neurons, 0.0);
    
    weightVelocity.resize(inputSize, std::vector<double>(neurons, 0.0));
    biasVelocity.resize(neurons, 0.0);
    
    activations.resize(neurons, 0.0);
    weightedSums.resize(neurons, 0.0);
    
    // Inicializar pesos aleatoriamente
    initializeWeights(weightInitRange);
}

void Layer::initializeWeights(double range) {
    // Gerador de números aleatórios com seed baseada no tempo
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-range, range);
    
    // Inicializar pesos com valores aleatórios uniformes
    for (int i = 0; i < inputSize; ++i) {
        for (int j = 0; j < neurons; ++j) {
            weights[i][j] = dis(gen);
        }
    }
    
    // Inicializar bias com valores pequenos aleatórios
    for (int j = 0; j < neurons; ++j) {
        bias[j] = dis(gen) * 0.1;  // Bias com amplitude reduzida
    }
}

std::vector<double> Layer::forward(const std::vector<double>& input) {
    if (input.size() != static_cast<size_t>(inputSize)) {
        throw std::invalid_argument(
            "Input size mismatch. Expected " + std::to_string(inputSize) +
            " but got " + std::to_string(input.size())
        );
    }
    
    // Guardar entrada para uso no backpropagation
    inputs = input;
    
    // Calcular saída de cada neurônio
    for (int j = 0; j < neurons; ++j) {
        // Soma ponderada: sum(input[i] * weight[i][j]) + bias[j]
        double sum = bias[j];
        
        for (int i = 0; i < inputSize; ++i) {
            sum += input[i] * weights[i][j];
        }
        
        weightedSums[j] = sum;
        
        // Aplicar função de ativação
        activations[j] = activationFunction->activate(sum);
    }
    
    return activations;
}

std::vector<double> Layer::backward(const std::vector<double>& outputGradients) {
    if (outputGradients.size() != static_cast<size_t>(neurons)) {
        throw std::invalid_argument("Output gradients size mismatch");
    }
    
    // Calcular gradientes para propagar para a camada anterior
    std::vector<double> inputGradients(inputSize, 0.0);
    
    // Para cada neurônio nesta camada
    for (int j = 0; j < neurons; ++j) {
        // Calcular gradiente local: output_gradient * derivative(activation)
        double derivative = activationFunction->derivative(activations[j]);
        double localGradient = outputGradients[j] * derivative;
        
        // Atualizar gradientes dos pesos e bias
        biasGradients[j] = localGradient;
        
        for (int i = 0; i < inputSize; ++i) {
            // Gradiente do peso: local_gradient * input
            weightGradients[i][j] = localGradient * inputs[i];
            
            // Propagar gradiente para a camada anterior
            inputGradients[i] += localGradient * weights[i][j];
        }
    }
    
    return inputGradients;
}

void Layer::updateWeights(double learningRate, double momentum) {
    // Atualizar pesos usando gradiente descendente com momentum
    for (int i = 0; i < inputSize; ++i) {
        for (int j = 0; j < neurons; ++j) {
            // Calcular mudança com momentum:
            // velocity = momentum * velocity - learningRate * gradient
            weightVelocity[i][j] = momentum * weightVelocity[i][j] 
                                  - learningRate * weightGradients[i][j];
            
            // Atualizar peso
            weights[i][j] += weightVelocity[i][j];
            
            // Resetar gradiente
            weightGradients[i][j] = 0.0;
        }
    }
    
    // Atualizar bias
    for (int j = 0; j < neurons; ++j) {
        biasVelocity[j] = momentum * biasVelocity[j] 
                         - learningRate * biasGradients[j];
        bias[j] += biasVelocity[j];
        biasGradients[j] = 0.0;
    }
}

void Layer::setWeights(const std::vector<std::vector<double>>& newWeights) {
    if (newWeights.size() != static_cast<size_t>(inputSize) ||
        (newWeights.size() > 0 && newWeights[0].size() != static_cast<size_t>(neurons))) {
        throw std::invalid_argument("Weight dimensions mismatch");
    }
    weights = newWeights;
}

void Layer::setBias(const std::vector<double>& newBias) {
    if (newBias.size() != static_cast<size_t>(neurons)) {
        throw std::invalid_argument("Bias dimensions mismatch");
    }
    bias = newBias;
}
