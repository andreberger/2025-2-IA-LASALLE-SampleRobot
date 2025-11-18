#include "../include/NeuralCollisionAvoidance.h"
#include "../include/neuralnetwork/ActivationFunction.h"
#include "../include/Config.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

NeuralCollisionAvoidance::NeuralCollisionAvoidance(PioneerRobot* _robo)
    : robo(_robo),
      decisionCount(0),
      rightDecisions(0),
      leftDecisions(0),
      forwardDecisions(0),
      backwardDecisions(0),
      stopDecisions(0) {
    
    // Inicializar array de sensores
    for (int i = 0; i < 8; ++i) {
        sonar[i] = 0;
    }
}

bool NeuralCollisionAvoidance::initializeNetwork(const std::string& weightsFile) {
    try {
        std::cout << "\n========================================" << std::endl;
        std::cout << "Inicializando Collision Avoidance Neural" << std::endl;
        std::cout << "========================================\n" << std::endl;
        
        // Criar rede neural
        // Input: 4 (direita, esquerda, frente, trás)
        // Output: 1 (ação codificada)
        network = std::make_unique<NeuralNetwork>(4, 1, 0.3, 0.9);
        
        // Adicionar camada oculta com 5 neurônios (baseado no exemplo original)
        network->addHiddenLayer(5, std::make_shared<SigmoidActivation>(), 0.5);
        
        // Finalizar com camada de saída usando sigmoid
        network->finalize(std::make_shared<SigmoidActivation>(), 0.5);
        
        std::cout << network->getArchitectureInfo() << "\n" << std::endl;
        
        // Tentar carregar pesos pré-treinados
        if (!weightsFile.empty()) {
            std::cout << "Tentando carregar pesos de: " << weightsFile << std::endl;
            if (network->loadWeights(weightsFile)) {
                std::cout << "✓ Pesos carregados com sucesso!" << std::endl;
                return true;
            } else {
                std::cout << "⚠ Não foi possível carregar pesos. Iniciando treinamento..." << std::endl;
        }
    }
    
    // Treinar a rede se não houver pesos pré-treinados
    std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>> trainingData = createTrainingData();
    std::vector<std::vector<double>>& trainingInputs = trainingData.first;
    std::vector<std::vector<double>>& trainingTargets = trainingData.second;
    
    std::cout << "\nIniciando treinamento da rede neural..." << std::endl;
    network->trainBatch(trainingInputs, trainingTargets, 
                        100000, 0.004, true);
    
    // Validar a rede
    std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>> validationData = createValidationData();
    std::vector<std::vector<double>>& validationInputs = validationData.first;
    std::vector<std::vector<double>>& validationTargets = validationData.second;
    network->validate(validationInputs, validationTargets, true);
    
    std::cout << "\n✓ Rede neural inicializada e treinada com sucesso!" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    return true;    } catch (const std::exception& e) {
        std::cerr << "Erro ao inicializar rede neural: " << e.what() << std::endl;
        return false;
    }
}

std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>>
NeuralCollisionAvoidance::createTrainingData() {
    
    // Dataset baseado no SimpleNeuralNetwork.c original, mas expandido
    // Formato: {direita, esquerda, frente, tras} -> {ação}
    // 1 = livre, 0 = obstruído
    
    std::vector<std::vector<double>> inputs = {
        // 1 direção livre
        {1, 0, 0, 0},  // Apenas direita livre -> virar direita
        {0, 1, 0, 0},  // Apenas esquerda livre -> virar esquerda
        {0, 0, 1, 0},  // Apenas frente livre -> seguir frente
        {0, 0, 0, 1},  // Apenas trás livre -> ir para trás
        
        // 2 direções livres
        {0, 0, 1, 1},  // Frente e trás livres -> preferir frente
        {1, 1, 0, 0},  // Direita e esquerda livres -> preferir direita
        {0, 1, 1, 0},  // Esquerda e frente livres -> preferir frente
        {1, 0, 0, 1},  // Direita e trás livres -> preferir direita
        {1, 0, 1, 0},  // Direita e frente livres -> preferir frente
        {0, 1, 0, 1},  // Esquerda e trás livres -> preferir esquerda
        
        // 3 direções livres
        {0, 1, 1, 1},  // Tudo livre exceto direita -> preferir frente
        {1, 0, 1, 1},  // Tudo livre exceto esquerda -> preferir frente
        {1, 1, 0, 1},  // Tudo livre exceto frente -> preferir direita
        {1, 1, 1, 0},  // Tudo livre exceto trás -> preferir frente
        
        // 4 direções livres
        {1, 1, 1, 1},  // Tudo livre -> seguir frente
        
        // Nenhuma direção livre
        {0, 0, 0, 0}   // Tudo bloqueado -> parar
    };
    
    std::vector<std::vector<double>> targets = {
        // 1 direção livre
        {0.53},  // DIREITA
        {0.59},  // ESQUERDA
        {0.65},  // FRENTE
        {0.71},  // TRÁS
        
        // 2 direções livres
        {0.65},  // FRENTE
        {0.53},  // DIREITA
        {0.65},  // FRENTE
        {0.53},  // DIREITA
        {0.65},  // FRENTE
        {0.59},  // ESQUERDA
        
        // 3 direções livres
        {0.65},  // FRENTE
        {0.65},  // FRENTE
        {0.53},  // DIREITA
        {0.65},  // FRENTE
        
        // 4 direções livres
        {0.65},  // FRENTE
        
        // Nenhuma direção livre
        {0.77}   // PARAR
    };
    
    return {inputs, targets};
}

std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>>
NeuralCollisionAvoidance::createValidationData() {
    
    // Dataset menor para validação
    std::vector<std::vector<double>> inputs = {
        {1, 0, 0, 0},  // Direita livre
        {0, 1, 0, 0},  // Esquerda livre
        {0, 0, 1, 0},  // Frente livre
        {0, 0, 0, 1},  // Trás livre
        {1, 1, 1, 1},  // Tudo livre
        {0, 0, 0, 0}   // Tudo bloqueado
    };
    
    std::vector<std::vector<double>> targets = {
        {0.53},  // DIREITA
        {0.59},  // ESQUERDA
        {0.65},  // FRENTE
        {0.71},  // TRÁS
        {0.65},  // FRENTE
        {0.77}   // PARAR
    };
    
    return {inputs, targets};
}

std::vector<double> NeuralCollisionAvoidance::normalizeSensorData(const int* sensorValues) {
    // Agregar sensores em 4 direções principais
    
    // Direita: sensor 0 (lateral direita) + sensores 1,2 (diagonais direita)
    int rightSide = std::max({sensorValues[0], sensorValues[1], sensorValues[2]});
    
    // Esquerda: sensor 7 (lateral esquerda) + sensores 5,6 (diagonais esquerda)
    int leftSide = std::max({sensorValues[7], sensorValues[6], sensorValues[5]});
    
    // Frente: sensores 3 e 4 (frente central)
    int frontSide = std::max(sensorValues[3], sensorValues[4]);
    
    // Trás: não temos sensores traseiros, então usamos uma média dos laterais
    // ou consideramos sempre livre se os laterais estiverem livres
    int backSide = (leftSide + rightSide) / 2;
    
    // Normalizar: 1 = livre (> threshold), 0 = obstruído (<= threshold)
    std::vector<double> normalized(4);
    normalized[0] = (rightSide > NEAR_THRESHOLD) ? 1.0 : 0.0;  // Direita
    normalized[1] = (leftSide > NEAR_THRESHOLD) ? 1.0 : 0.0;   // Esquerda
    normalized[2] = (frontSide > NEAR_THRESHOLD) ? 1.0 : 0.0;  // Frente
    normalized[3] = (backSide > NEAR_THRESHOLD) ? 1.0 : 0.0;   // Trás
    
    return normalized;
}

void NeuralCollisionAvoidance::executeAction(double networkOutput) {
    decisionCount++;
    
    std::string actionName;
    
    // Interpretar saída da rede e executar ação correspondente
    if (networkOutput >= ACTION_RIGHT_MIN && networkOutput < ACTION_RIGHT_MAX) {
        // VIRAR DIREITA
        if (robo->robot.isHeadingDone()) {
            robo->Rotaciona(ROTATION_ANGLE, 2, VELOCITY_ROTATION);
            rightDecisions++;
            actionName = "DIREITA";
        } else {
            actionName = "AGUARDANDO ROTAÇÃO";
        }
    }
    else if (networkOutput >= ACTION_LEFT_MIN && networkOutput < ACTION_LEFT_MAX) {
        // VIRAR ESQUERDA
        if (robo->robot.isHeadingDone()) {
            robo->Rotaciona(ROTATION_ANGLE, 1, VELOCITY_ROTATION);
            leftDecisions++;
            actionName = "ESQUERDA";
        } else {
            actionName = "AGUARDANDO ROTAÇÃO";
        }
    }
    else if (networkOutput >= ACTION_FORWARD_MIN && networkOutput < ACTION_FORWARD_MAX) {
        // SEGUIR EM FRENTE
        if (robo->robot.isHeadingDone()) {
            robo->Move(VELOCITY_MOVE, VELOCITY_MOVE);
            forwardDecisions++;
            actionName = "FRENTE";
        } else {
            actionName = "AGUARDANDO ROTAÇÃO";
        }
    }
    else if (networkOutput >= ACTION_BACKWARD_MIN && networkOutput < ACTION_BACKWARD_MAX) {
        // MOVER PARA TRÁS
        if (robo->robot.isHeadingDone()) {
            robo->Move(-VELOCITY_MOVE, -VELOCITY_MOVE);
            backwardDecisions++;
            actionName = "TRÁS";
        } else {
            actionName = "AGUARDANDO ROTAÇÃO";
        }
    }
    else if (networkOutput >= ACTION_STOP_MIN && networkOutput < ACTION_STOP_MAX) {
        // PARAR
        robo->pararMovimento();
        stopDecisions++;
        actionName = "PARAR";
    }
    else {
        // Valor fora dos intervalos esperados - comportamento padrão
        std::cout << "⚠ Saída inesperada da rede: " << networkOutput << std::endl;
        robo->Move(VELOCITY_MOVE, VELOCITY_MOVE);
        forwardDecisions++;
        actionName = "FRENTE (padrão)";
    }
    
    // Log da decisão
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Decisão #" << decisionCount << " | Saída NN: " << networkOutput 
             << " | Ação: " << actionName << std::endl;
}

void* NeuralCollisionAvoidance::runThread(void*) {
    std::cout << "Thread de Collision Avoidance Neural iniciada." << std::endl;
    
    while (this->getRunningWithLock()) {
        myMutex.lock();
        
        // Obter leituras dos sensores
        robo->getAllSonar(sonar);
        
        // Normalizar dados dos sensores
        std::vector<double> normalizedInput = normalizeSensorData(sonar);
        
        // Obter predição da rede neural
        std::vector<double> output = network->predict(normalizedInput);
        
        // Executar ação baseada na predição
        executeAction(output[0]);
        
        myMutex.unlock();
        
        // Pequeno delay para não sobrecarregar o sistema
        ArUtil::sleep(100);
    }
    
    // Exibir estatísticas ao finalizar
    printStatistics();
    
    ArLog::log(ArLog::Normal, "Thread de Collision Avoidance Neural encerrada.");
    return nullptr;
}

void NeuralCollisionAvoidance::waitOnCondition() {
    myCondition.wait();
}

void NeuralCollisionAvoidance::lockMutex() {
    myMutex.lock();
}

void NeuralCollisionAvoidance::unlockMutex() {
    myMutex.unlock();
}

bool NeuralCollisionAvoidance::saveNetworkWeights(const std::string& filename) {
    if (network) {
        return network->saveWeights(filename);
    }
    return false;
}

void NeuralCollisionAvoidance::printStatistics() const {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Estatísticas de Decisões" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Total de decisões: " << decisionCount << std::endl;
    
    if (decisionCount > 0) {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "  Frente:    " << forwardDecisions 
                 << " (" << (100.0 * forwardDecisions / decisionCount) << "%)" << std::endl;
        std::cout << "  Direita:   " << rightDecisions 
                 << " (" << (100.0 * rightDecisions / decisionCount) << "%)" << std::endl;
        std::cout << "  Esquerda:  " << leftDecisions 
                 << " (" << (100.0 * leftDecisions / decisionCount) << "%)" << std::endl;
        std::cout << "  Trás:      " << backwardDecisions 
                 << " (" << (100.0 * backwardDecisions / decisionCount) << "%)" << std::endl;
        std::cout << "  Parar:     " << stopDecisions 
                 << " (" << (100.0 * stopDecisions / decisionCount) << "%)" << std::endl;
    }
    
    std::cout << "========================================\n" << std::endl;
}

int NeuralCollisionAvoidance::determineBestDirection() const {
    // Calcular soma ponderada para cada lado
    int sumRight = (sonar[3] * 3) + ((sonar[2] + sonar[1]) * 2) + sonar[0];
    int sumLeft = (sonar[4] * 3) + ((sonar[5] + sonar[6]) * 2) + sonar[7];
    
    // Retornar lado com mais espaço livre
    return (sumLeft > sumRight) ? 1 : 2;  // 1 = esquerda, 2 = direita
}
