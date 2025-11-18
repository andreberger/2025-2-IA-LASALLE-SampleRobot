#!/bin/bash

# Script auxiliar para gerenciar o projeto Neural Collision Avoidance
# Autor: [Seu Nome]
# Data: Novembro 2025

set -e  # Sair em caso de erro

# Cores para output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Banner
print_banner() {
    echo -e "${BLUE}"
    echo "╔════════════════════════════════════════════════════╗"
    echo "║   Neural Collision Avoidance - Helper Script      ║"
    echo "╚════════════════════════════════════════════════════╝"
    echo -e "${NC}"
}

# Função de help
show_help() {
    print_banner
    echo "Uso: ./run.sh [comando]"
    echo ""
    echo "Comandos disponíveis:"
    echo "  setup          - Compila todo o projeto"
    echo "  train          - Treina a rede neural e salva pesos"
    echo "  run            - Executa o robô com rede neural"
    echo "  run-trained    - Executa com pesos pré-treinados"
    echo "  test           - Compila e testa tudo"
    echo "  clean          - Limpa arquivos compilados"
    echo "  stats          - Mostra estatísticas do git"
    echo "  help           - Mostra esta mensagem"
    echo ""
    echo "Exemplos:"
    echo "  ./run.sh setup         # Primeira vez"
    echo "  ./run.sh train         # Treinar rede"
    echo "  ./run.sh run-trained   # Usar pesos salvos"
}

# Setup inicial
setup() {
    print_banner
    echo -e "${YELLOW}Compilando projeto...${NC}"
    make clean
    make all
    echo -e "${GREEN}✓ Compilação concluída!${NC}"
    echo ""
    echo "Próximos passos:"
    echo "  1. ./run.sh train      # Treinar rede neural"
    echo "  2. Inicie MobileSim"
    echo "  3. ./run.sh run        # Executar robô"
}

# Treinar rede
train() {
    print_banner
    echo -e "${YELLOW}Verificando compilação...${NC}"
    
    if [ ! -f "build/train_network" ]; then
        echo -e "${RED}Programa de treinamento não encontrado. Compilando...${NC}"
        make train
    fi
    
    echo -e "${YELLOW}Iniciando treinamento da rede neural...${NC}"
    echo ""
    ./build/train_network
    
    echo ""
    echo -e "${GREEN}✓ Treinamento concluído!${NC}"
    
    if [ -f "trained_weights.json" ]; then
        echo -e "${GREEN}✓ Pesos salvos em: trained_weights.json${NC}"
    fi
}

# Executar robô
run_robot() {
    print_banner
    
    # Verificar se MobileSim está rodando
    if ! pgrep -x "MobileSim" > /dev/null; then
        echo -e "${YELLOW}⚠ MobileSim não está rodando!${NC}"
        echo "Inicie MobileSim em outro terminal:"
        echo "  MobileSim"
        echo ""
        read -p "Pressione Enter quando MobileSim estiver rodando..."
    fi
    
    echo -e "${YELLOW}Verificando compilação...${NC}"
    
    if [ ! -f "build/main" ]; then
        echo -e "${RED}Programa principal não encontrado. Compilando...${NC}"
        make robot
    fi
    
    echo -e "${YELLOW}Executando robô...${NC}"
    echo ""
    ./build/main "$@"
}

# Executar com pesos treinados
run_trained() {
    print_banner
    
    if [ ! -f "trained_weights.json" ]; then
        echo -e "${RED}✗ Pesos treinados não encontrados!${NC}"
        echo "Execute primeiro: ./run.sh train"
        exit 1
    fi
    
    echo -e "${GREEN}✓ Usando pesos de: trained_weights.json${NC}"
    run_robot "trained_weights.json"
}

# Teste completo
test_all() {
    print_banner
    echo -e "${YELLOW}Executando testes completos...${NC}"
    echo ""
    
    echo "1. Limpando build..."
    make clean
    
    echo "2. Compilando tudo..."
    make all
    
    echo "3. Testando programa de treinamento..."
    if ./build/train_network test_weights.json > /dev/null 2>&1; then
        echo -e "${GREEN}   ✓ Treinamento OK${NC}"
    else
        echo -e "${RED}   ✗ Erro no treinamento${NC}"
        exit 1
    fi
    
    echo "4. Verificando arquivos gerados..."
    if [ -f "test_weights.json" ]; then
        echo -e "${GREEN}   ✓ Pesos salvos corretamente${NC}"
        rm test_weights.json
    else
        echo -e "${RED}   ✗ Pesos não foram salvos${NC}"
        exit 1
    fi
    
    echo ""
    echo -e "${GREEN}✓ Todos os testes passaram!${NC}"
}

# Limpar
clean() {
    print_banner
    echo -e "${YELLOW}Limpando arquivos compilados...${NC}"
    make clean
    echo -e "${GREEN}✓ Build limpo${NC}"
}

# Estatísticas Git
show_stats() {
    print_banner
    echo -e "${BLUE}Estatísticas do Repositório${NC}"
    echo ""
    
    echo "Commits por autor:"
    git shortlog -sn --all
    
    echo ""
    echo "Últimos commits:"
    git log --oneline -10
    
    echo ""
    echo "Arquivos modificados:"
    git diff --stat HEAD~10
}

# Verificar argumentos
if [ $# -eq 0 ]; then
    show_help
    exit 0
fi

# Processar comando
case "$1" in
    setup)
        setup
        ;;
    train)
        train
        ;;
    run)
        shift
        run_robot "$@"
        ;;
    run-trained)
        run_trained
        ;;
    test)
        test_all
        ;;
    clean)
        clean
        ;;
    stats)
        show_stats
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        echo -e "${RED}Comando desconhecido: $1${NC}"
        echo ""
        show_help
        exit 1
        ;;
esac
