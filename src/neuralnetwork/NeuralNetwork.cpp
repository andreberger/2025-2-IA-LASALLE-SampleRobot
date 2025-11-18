#include "../include/neuralnetwork/NeuralNetwork.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <stdexcept>

NeuralNetwork::NeuralNetwork(int inputSize, int outputSize,
                             double learningRate, double momentum)
    : inputSize(inputSize),
      outputSize(outputSize),
      learningRate(learningRate),
      momentum(momentum),
      lastError(0.0),
      trainingIterations(0) {
    
    if (inputSize <= 0 || outputSize <= 0) {
        throw std::invalid_argument("Input and output sizes must be positive");
    }
}

void NeuralNetwork::addHiddenLayer(int neurons,
                                  std::shared_ptr<ActivationFunction> activationFunc,
                                  double weightInitRange) {
    // Determinar o tamanho da entrada para esta camada
    int layerInputSize = layers.empty() ? inputSize : layers.back()->getOutputSize();
    
    // Criar e adicionar a nova camada
    auto layer = std::make_shared<Layer>(layerInputSize, neurons, 
                                         activationFunc, weightInitRange);
    layers.push_back(layer);
}

void NeuralNetwork::finalize(std::shared_ptr<ActivationFunction> activationFunc,
                             double weightInitRange) {
    // Adicionar camada de saída
    int layerInputSize = layers.empty() ? inputSize : layers.back()->getOutputSize();
    
    auto outputLayer = std::make_shared<Layer>(layerInputSize, outputSize,
                                               activationFunc, weightInitRange);
    layers.push_back(outputLayer);
}

std::vector<double> NeuralNetwork::predict(const std::vector<double>& input) {
    if (input.size() != static_cast<size_t>(inputSize)) {
        throw std::invalid_argument(
            "Input size mismatch. Expected " + std::to_string(inputSize) +
            " but got " + std::to_string(input.size())
        );
    }
    
    if (layers.empty()) {
        throw std::runtime_error("Network not finalized. Call finalize() first.");
    }
    
    // Forward propagation através de todas as camadas
    std::vector<double> output = input;
    
    for (auto& layer : layers) {
        output = layer->forward(output);
    }
    
    return output;
}

double NeuralNetwork::train(const std::vector<double>& input,
                           const std::vector<double>& target) {
    if (target.size() != static_cast<size_t>(outputSize)) {
        throw std::invalid_argument("Target size mismatch");
    }
    
    // Forward propagation
    std::vector<double> output = predict(input);
    
    // Calcular erro
    double error = calculateError(output, target);
    lastError = error;
    
    // Backward propagation
    std::vector<double> gradients = calculateOutputGradients(output, target);
    
    // Propagar gradientes de trás para frente
    for (int i = layers.size() - 1; i >= 0; --i) {
        gradients = layers[i]->backward(gradients);
    }
    
    // Atualizar pesos de todas as camadas
    for (auto& layer : layers) {
        layer->updateWeights(learningRate, momentum);
    }
    
    trainingIterations++;
    
    return error;
}

int NeuralNetwork::trainBatch(const std::vector<std::vector<double>>& inputs,
                              const std::vector<std::vector<double>>& targets,
                              int epochs,
                              double errorThreshold,
                              bool verbose) {
    if (inputs.size() != targets.size()) {
        throw std::invalid_argument("Number of inputs and targets must match");
    }
    
    int numPatterns = inputs.size();
    std::vector<int> indices(numPatterns);
    
    // Inicializar índices para embaralhamento
    for (int i = 0; i < numPatterns; ++i) {
        indices[i] = i;
    }
    
    if (verbose) {
        std::cout << "\n========================================" << std::endl;
        std::cout << "Iniciando treinamento da rede neural" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Padrões de treinamento: " << numPatterns << std::endl;
        std::cout << "Épocas máximas: " << epochs << std::endl;
        std::cout << "Limiar de erro: " << errorThreshold << std::endl;
        std::cout << getArchitectureInfo() << std::endl;
        std::cout << "========================================\n" << std::endl;
    }
    
    int epoch;
    for (epoch = 1; epoch <= epochs; ++epoch) {
        // Embaralhar padrões de treinamento para evitar mínimos locais
        std::random_shuffle(indices.begin(), indices.end());
        
        double totalError = 0.0;
        
        // Treinar com cada padrão
        for (int idx : indices) {
            double error = train(inputs[idx], targets[idx]);
            totalError += error;
        }
        
        double avgError = totalError / numPatterns;
        
        // Exibir progresso periodicamente
        if (verbose && (epoch % 1000 == 0 || epoch == 1)) {
            std::cout << std::fixed << std::setprecision(6);
            std::cout << "Época " << std::setw(6) << epoch 
                     << " | Erro médio: " << avgError << std::endl;
        }
        
        // Verificar convergência
        if (avgError < errorThreshold) {
            if (verbose) {
                std::cout << "\n✓ Convergência alcançada na época " << epoch << std::endl;
                std::cout << "  Erro final: " << avgError << std::endl;
            }
            break;
        }
    }
    
    if (verbose) {
        if (epoch > epochs) {
            std::cout << "\n⚠ Número máximo de épocas atingido" << std::endl;
        }
        std::cout << "\nTreinamento concluído." << std::endl;
        std::cout << "========================================\n" << std::endl;
    }
    
    return epoch;
}

double NeuralNetwork::validate(const std::vector<std::vector<double>>& inputs,
                              const std::vector<std::vector<double>>& targets,
                              bool verbose) {
    if (inputs.size() != targets.size()) {
        throw std::invalid_argument("Number of inputs and targets must match");
    }
    
    double totalError = 0.0;
    
    if (verbose) {
        std::cout << "\n========================================" << std::endl;
        std::cout << "Validação da rede neural" << std::endl;
        std::cout << "========================================" << std::endl;
    }
    
    for (size_t i = 0; i < inputs.size(); ++i) {
        std::vector<double> output = predict(inputs[i]);
        double error = calculateError(output, targets[i]);
        totalError += error;
        
        if (verbose) {
            std::cout << "Padrão " << (i + 1) << ":" << std::endl;
            
            std::cout << "  Entrada:   [";
            for (size_t j = 0; j < inputs[i].size(); ++j) {
                std::cout << std::fixed << std::setprecision(2) << inputs[i][j];
                if (j < inputs[i].size() - 1) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
            
            std::cout << "  Esperado:  [";
            for (size_t j = 0; j < targets[i].size(); ++j) {
                std::cout << std::fixed << std::setprecision(4) << targets[i][j];
                if (j < targets[i].size() - 1) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
            
            std::cout << "  Predição:  [";
            for (size_t j = 0; j < output.size(); ++j) {
                std::cout << std::fixed << std::setprecision(4) << output[j];
                if (j < output.size() - 1) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
            
            std::cout << "  Erro:      " << std::fixed << std::setprecision(6) 
                     << error << "\n" << std::endl;
        }
    }
    
    double avgError = totalError / inputs.size();
    
    if (verbose) {
        std::cout << "Erro médio de validação: " << std::fixed 
                 << std::setprecision(6) << avgError << std::endl;
        std::cout << "========================================\n" << std::endl;
    }
    
    return avgError;
}

bool NeuralNetwork::saveWeights(const std::string& filename) const {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Erro ao abrir arquivo para salvar: " << filename << std::endl;
            return false;
        }
        
        // Salvar arquitetura básica
        file << "{\n";
        file << "  \"architecture\": {\n";
        file << "    \"inputSize\": " << inputSize << ",\n";
        file << "    \"outputSize\": " << outputSize << ",\n";
        file << "    \"numLayers\": " << layers.size() << "\n";
        file << "  },\n";
        
        // Salvar hiperparâmetros
        file << "  \"hyperparameters\": {\n";
        file << "    \"learningRate\": " << learningRate << ",\n";
        file << "    \"momentum\": " << momentum << "\n";
        file << "  },\n";
        
        // Salvar cada camada
        file << "  \"layers\": [\n";
        for (size_t l = 0; l < layers.size(); ++l) {
            const auto& layer = layers[l];
            const auto& weights = layer->getWeights();
            const auto& bias = layer->getBias();
            
            file << "    {\n";
            file << "      \"inputSize\": " << layer->getInputSize() << ",\n";
            file << "      \"neurons\": " << layer->getOutputSize() << ",\n";
            file << "      \"activation\": \"" << layer->getActivationName() << "\",\n";
            
            // Salvar pesos
            file << "      \"weights\": [\n";
            for (size_t i = 0; i < weights.size(); ++i) {
                file << "        [";
                for (size_t j = 0; j < weights[i].size(); ++j) {
                    file << std::setprecision(10) << weights[i][j];
                    if (j < weights[i].size() - 1) file << ", ";
                }
                file << "]";
                if (i < weights.size() - 1) file << ",";
                file << "\n";
            }
            file << "      ],\n";
            
            // Salvar bias
            file << "      \"bias\": [";
            for (size_t i = 0; i < bias.size(); ++i) {
                file << std::setprecision(10) << bias[i];
                if (i < bias.size() - 1) file << ", ";
            }
            file << "]\n";
            
            file << "    }";
            if (l < layers.size() - 1) file << ",";
            file << "\n";
        }
        file << "  ]\n";
        file << "}\n";
        
        file.close();
        std::cout << "✓ Pesos salvos com sucesso em: " << filename << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Erro ao salvar pesos: " << e.what() << std::endl;
        return false;
    }
}

bool NeuralNetwork::loadWeights(const std::string& filename) {
    // Nota: Implementação simplificada - uma versão completa usaria uma biblioteca JSON
    // Por enquanto, esta é uma implementação básica para demonstração
    
    std::cout << "⚠ Função loadWeights() ainda não implementada completamente." << std::endl;
    std::cout << "  Para carregar pesos, será necessário implementar parser JSON." << std::endl;
    std::cout << "  Sugestão: usar biblioteca nlohmann/json ou similar." << std::endl;
    
    return false;
}

std::string NeuralNetwork::getArchitectureInfo() const {
    std::ostringstream oss;
    oss << "Arquitetura da rede:\n";
    oss << "  Entrada: " << inputSize << " neurônios\n";
    
    for (size_t i = 0; i < layers.size(); ++i) {
        if (i == layers.size() - 1) {
            oss << "  Saída:   ";
        } else {
            oss << "  Oculta " << (i + 1) << ": ";
        }
        oss << layers[i]->getOutputSize() << " neurônios ("
            << layers[i]->getActivationName() << ")\n";
    }
    
    oss << "  Taxa de aprendizado: " << learningRate << "\n";
    oss << "  Momentum: " << momentum;
    
    return oss.str();
}

double NeuralNetwork::calculateError(const std::vector<double>& output,
                                    const std::vector<double>& target) const {
    double error = 0.0;
    
    // Erro quadrático médio: E = 0.5 * sum((target - output)^2)
    for (size_t i = 0; i < output.size(); ++i) {
        double diff = target[i] - output[i];
        error += 0.5 * diff * diff;
    }
    
    return error;
}

std::vector<double> NeuralNetwork::calculateOutputGradients(
    const std::vector<double>& output,
    const std::vector<double>& target) const {
    
    std::vector<double> gradients(output.size());
    
    // Gradiente para erro quadrático: gradient = -(target - output)
    for (size_t i = 0; i < output.size(); ++i) {
        gradients[i] = -(target[i] - output[i]);
    }
    
    return gradients;
}
