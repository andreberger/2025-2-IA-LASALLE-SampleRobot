/**
 * @file main_neural.cpp
 * @brief Programa principal usando Collision Avoidance Neural
 * 
 * Este programa utiliza a rede neural para controlar o robô no simulador.
 * Pode carregar pesos pré-treinados ou treinar uma nova rede.
 * 
 * Uso:
 *   ./build/main [weights_file]
 * 
 * Se weights_file for fornecido, tentará carregar pesos pré-treinados.
 * Caso contrário, treinará uma nova rede.
 */

#include "ClassRobo.h"
#include "Aria.h"
#include "Config.h"
#include "NeuralCollisionAvoidance.h"
#include "Sonarthread.h"
#include <iostream>
#include <string>

PioneerRobot* robo;

int main(int argc, char** argv) {
    std::cout << "\n╔════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║   COLLISION AVOIDANCE NEURAL                       ║" << std::endl;
    std::cout << "║   Robô Pioneer com Rede Neural                     ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════╝\n" << std::endl;
    
    int sucesso;
    
    // Inicializar conexão com o robô (simulador ou real)
    std::cout << "Conectando ao robô..." << std::endl;
    robo = new PioneerRobot(ConexaoSimulacao, "", &sucesso);
    
    if (!sucesso) {
        std::cerr << "✗ Falha ao conectar ao robô!" << std::endl;
        std::cerr << "  Verifique se o MobileSim está rodando." << std::endl;
        return 1;
    }
    
    std::cout << "✓ Robô conectado com sucesso!\n" << std::endl;
    
    // Arquivo de pesos (se fornecido via linha de comando)
    std::string weightsFile = "";
    if (argc > 1) {
        weightsFile = argv[1];
        std::cout << "Arquivo de pesos especificado: " << weightsFile << std::endl;
    }
    
    // Criar threads
    ArLog::log(ArLog::Normal, "Criando threads do sistema...");
    
    // Thread de leitura dos sensores sonar
    SonarThread sonarReadingThread(robo);
    
    // Thread de collision avoidance com rede neural
    NeuralCollisionAvoidance neuralCollisionAvoidance(robo);
    
    // Inicializar rede neural
    std::cout << "\nInicializando sistema de Collision Avoidance Neural..." << std::endl;
    if (!neuralCollisionAvoidance.initializeNetwork(weightsFile)) {
        std::cerr << "✗ Falha ao inicializar rede neural!" << std::endl;
        delete robo;
        return 1;
    }
    
    // Opção de salvar pesos após treinamento
    std::cout << "\nDeseja salvar os pesos da rede? (s/n): ";
    char resposta;
    std::cin >> resposta;
    
    if (resposta == 's' || resposta == 'S') {
        std::string saveFile = "collision_avoidance_weights.json";
        std::cout << "Nome do arquivo (ou Enter para '" << saveFile << "'): ";
        std::cin.ignore();
        std::string input;
        std::getline(std::cin, input);
        if (!input.empty()) {
            saveFile = input;
        }
        neuralCollisionAvoidance.saveNetworkWeights(saveFile);
    }
    
    // Iniciar threads
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "INICIANDO SISTEMA" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    ArLog::log(ArLog::Normal, "Iniciando thread de leitura de sensores...");
    sonarReadingThread.runAsync();
    
    ArLog::log(ArLog::Normal, "Iniciando thread de collision avoidance neural...");
    neuralCollisionAvoidance.runAsync();
    
    std::cout << "\n✓ Sistema em execução!" << std::endl;
    std::cout << "  O robô agora está sendo controlado pela rede neural." << std::endl;
    std::cout << "  Pressione Ctrl+C para encerrar.\n" << std::endl;
    
    // Aguardar até que o usuário encerre
    robo->robot.waitForRunExit();
    
    // Exibir estatísticas antes de sair
    neuralCollisionAvoidance.printStatistics();
    
    std::cout << "\nEncerrando programa..." << std::endl;
    delete robo;
    
    Aria::exit(0);
    return 0;
}
