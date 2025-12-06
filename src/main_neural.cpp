/**
 * @file main_neural.cpp
 * @brief Programa principal do sistema de navegação autônoma com rede neural
 * 
 * ==================================================================================
 * TRABALHO ACADÊMICO - SISTEMA DE NAVEGAÇÃO AUTÔNOMA COM REDE NEURAL
 * ==================================================================================
 * 
 * OBJETIVO:
 * Implementar navegação autônoma de robô móvel usando Inteligência Artificial
 * 
 * COMPONENTES DO SISTEMA:
 * 1. Robô Pioneer (físico ou simulado no MobileSim)
 * 2. Biblioteca ARIA (controle do robô)
 * 3. Sensores Sonar (8 sensores de proximidade)
 * 4. Rede Neural Artificial (tomada de decisão)
 * 5. Sistema de segurança com 3 zonas
 * 
 * FLUXO DE EXECUÇÃO:
 * 1. Conecta ao robô (real ou simulado)
 * 2. Carrega rede neural pré-treinada
 * 3. Inicia thread de leitura dos sensores
 * 4. Loop infinito:
 *    - Lê sensores sonar
 *    - Normaliza dados
 *    - Rede neural decide ação
 *    - Sistema de segurança valida
 *    - Executa movimento
 * 5. Ao encerrar (Ctrl+C), mostra estatísticas
 * 
 * DIFERENCIAL:
 * Sistema híbrido que combina IA (rede neural) com regras de segurança
 * - IA decide em condições normais (> 600mm de distância)
 * - Regras fixas garantem segurança em emergências (< 250mm)
 * 
 * USO:
 *   ./build/main_neural [arquivo_de_pesos]
 * 
 * EXEMPLO:
 *   ./build/main_neural trained_weights.json
 * 
 * Se o arquivo de pesos não for fornecido, treinará uma nova rede
 * (não recomendado - melhor usar pesos pré-treinados para consistência)
 * 
 * @author Grupo IA - La Salle
 * @date Novembro/Dezembro 2025
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
    std::cout << "║                                                    ║" << std::endl;
    std::cout << "║   TRABALHO: Navegação Autônoma com IA              ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════╝\n" << std::endl;
    
    // ===== ETAPA 1: OBTER ARQUIVO DE PESOS =====
    // Permite carregar rede pré-treinada para comportamento consistente
    int sucesso;
    
    // Inicializar conexão com o robô (simulador ou real)
    std::cout << "Conectando ao robô..." << std::endl;
    robo = new PioneerRobot(ConexaoSimulacao, "", &sucesso);
    
    if (!sucesso) {
        std::cerr << "✗ Falha ao conectar ao robô!" << std::endl;
        std::cerr << "  Verifique se o MobileSim está rodando." << std::endl;
        std::cerr << "  Execute: MobileSim -r p3dx" << std::endl;
        return 1;
    }
    
    std::cout << "✓ Robô conectado com sucesso!\n" << std::endl;
    
    // Arquivo de pesos (se fornecido via linha de comando)
    std::string weightsFile = "";
    if (argc > 1) {
        weightsFile = argv[1];
        std::cout << "Arquivo de pesos especificado: " << weightsFile << std::endl;
    }
    
    // ===== ETAPA 3: CRIAR THREADS DO SISTEMA =====
    // MULTITHREADING: Permite processos simultâneos
    // - Thread principal: controle geral
    // - Thread sonar: leitura contínua dos sensores
    // - Thread IA: processamento neural e decisões
    ArLog::log(ArLog::Normal, "Criando threads do sistema...");
    
    // Thread de leitura dos sensores sonar
    // Atualiza valores dos 8 sensores constantemente
    SonarThread sonarReadingThread(robo);
    
    // Thread de collision avoidance com rede neural
    // Processa dados e toma decisões de navegação
    NeuralCollisionAvoidance neuralCollisionAvoidance(robo);
    
    // ===== ETAPA 4: INICIALIZAR REDE NEURAL =====
    std::cout << "\nInicializando sistema de Collision Avoidance Neural..." << std::endl;
    std::cout << "ARQUITETURA: 4 entradas → 5 neurônios ocultos → 1 saída" << std::endl;
    std::cout << "DECISÕES: Direita, Esquerda, Frente, Trás, Parar" << std::endl;
    std::cout << "SEGURANÇA: Sistema de 3 zonas (livre/alerta/perigo)\n" << std::endl;
    
    if (!neuralCollisionAvoidance.initializeNetwork(weightsFile)) {
        std::cerr << "✗ Falha ao inicializar rede neural!" << std::endl;
        delete robo;
        return 1;
    }
    
    std::cout << "✓ Rede neural inicializada com sucesso!\n" << std::endl;
    
    // ===== ETAPA 5: OPÇÃO DE SALVAR PESOS =====
    // Útil se treinou uma nova rede e quer guardar os resultados
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
    
    // ===== ETAPA 6: INICIAR THREADS =====
    // Threads começam a executar em paralelo
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "INICIANDO SISTEMA" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    ArLog::log(ArLog::Normal, "Iniciando thread de leitura de sensores...");
    sonarReadingThread.runAsync();
    
    ArLog::log(ArLog::Normal, "Iniciando thread de collision avoidance neural...");
    neuralCollisionAvoidance.runAsync();
    
    std::cout << "\n✓ Sistema em execução!" << std::endl;
    std::cout << "  O robô agora está sendo controlado pela rede neural." << std::endl;
    std::cout << "\n📊 LOGS EM TEMPO REAL:" << std::endl;
    std::cout << "  [SENSORES] = Leituras dos sensores sonar" << std::endl;
    std::cout << "  [DECISÃO] = Output da rede e ação escolhida" << std::endl;
    std::cout << "  🛑 = Parada de emergência (< 250mm)" << std::endl;
    std::cout << "  ⚠️  = Obstáculo próximo (250-600mm)" << std::endl;
    std::cout << "  🔄 = Desvio forçado inteligente" << std::endl;
    std::cout << "  ⬆️ ➡️ ⬅️ = Movimento executado" << std::endl;
    std::cout << "\n  Pressione Ctrl+C para encerrar e ver estatísticas.\n" << std::endl;
    
    // Aguardar até que o usuário encerre
    robo->robot.waitForRunExit();
    
    // Exibir estatísticas antes de sair
    neuralCollisionAvoidance.printStatistics();
    
    std::cout << "\nEncerrando programa..." << std::endl;
    delete robo;
    
    Aria::exit(0);
    return 0;
}
