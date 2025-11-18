# 🚀 Guia de Início Rápido - Collision Avoidance Neural

## ⚡ Setup em 5 Minutos

### 1. Pré-requisitos
```bash
# Verifique se tem instalado:
g++ --version      # Deve ser g++ 5.0 ou superior
MobileSim          # Simulador do robô
```

### 2. Clone e Compile
```bash
cd /seu/diretorio/de/trabalho
git clone [seu-fork-url]
cd 2025-2-IA-LASALLE-SampleRobot
make
```

### 3. Treine a Rede (IMPORTANTE!)
```bash
# Isso vai criar o arquivo trained_weights.json
./build/train_network
```

Aguarde a mensagem:
```
✓ Convergência alcançada na época XXXXX
✓ Modelo treinado salvo com sucesso!
```

### 4. Execute o Robô
```bash
# Terminal 1: Inicie o simulador
MobileSim

# Terminal 2: Execute o robô
./build/main trained_weights.json
```

Pronto! O robô agora navega usando rede neural! 🎉

---

## 🎯 Para a Apresentação Final

### Checklist Antes da Apresentação

- [ ] **Compilar tudo**: `make clean && make`
- [ ] **Treinar rede**: `./build/train_network` (salva pesos)
- [ ] **Testar execução**: `./build/main trained_weights.json`
- [ ] **MobileSim rodando**: Mapa definido pelo professor
- [ ] **Código comentado**: Todos arquivos com documentação
- [ ] **Git log limpo**: Commits de todos membros visíveis

### Comandos Úteis

```bash
# Ver ajuda do Makefile
make help

# Compilar apenas treinamento
make train

# Compilar apenas robô
make robot

# Limpar e recompilar tudo
make clean && make all

# Ver commits de cada membro
git log --author="Nome" --oneline
```

---

## 🐛 Troubleshooting

### Erro: "cannot find -lAria"
```bash
# Reinstale ARIA
sudo dpkg -i libaria_2.9.4+ubuntu16_amd64.deb
```

### Erro: "Connection refused"
```bash
# Certifique-se que MobileSim está rodando
ps aux | grep MobileSim

# Se não estiver, inicie:
MobileSim &
```

### Rede não converge
```bash
# Aumente o número de épocas em train_network.cpp
# Linha ~XXX: trainBatch(..., 200000, ...)  // Era 100000
```

### Robô colide
```bash
# Verifique se carregou pesos treinados:
./build/main trained_weights.json

# Se não existir, treine primeiro:
./build/train_network
```

---

## 📊 Exemplo de Saída Esperada

### Treinamento
```
╔════════════════════════════════════════════════════╗
║   TREINAMENTO DA REDE NEURAL                       ║
╚════════════════════════════════════════════════════╝

Criando arquitetura da rede neural...
Arquitetura da rede:
  Entrada: 4 neurônios
  Oculta 1: 5 neurônios (Sigmoid)
  Saída:   1 neurônios (Sigmoid)
  Taxa de aprendizado: 0.3
  Momentum: 0.9

========================================
INICIANDO TREINAMENTO
========================================
Época      1 | Erro médio: 0.142567
Época   1000 | Erro médio: 0.045231
Época   2000 | Erro médio: 0.018456
...
Época  45231 | Erro médio: 0.003987

✓ Convergência alcançada na época 45231
  Erro final: 0.003987
```

### Execução do Robô
```
╔════════════════════════════════════════════════════╗
║   COLLISION AVOIDANCE NEURAL                       ║
╚════════════════════════════════════════════════════╝

Conectando ao robô...
✓ Robô conectado com sucesso!

✓ Rede neural inicializada e treinada com sucesso!
✓ Sistema em execução!

Decisão #1 | Saída NN: 0.6532 | Ação: FRENTE
Decisão #2 | Saída NN: 0.6498 | Ação: FRENTE
Decisão #3 | Saída NN: 0.5321 | Ação: DIREITA
Decisão #4 | Saída NN: 0.6587 | Ação: FRENTE
...
```

---

## 🎓 Demonstração de Conceitos IA

Este projeto demonstra:

### ✅ Aprendizado Supervisionado
- Dataset com pares (entrada, saída esperada)
- Função de erro comparando predição vs target
- Minimização iterativa do erro

### ✅ Backpropagation
- Cálculo de gradientes camada por camada
- Regra da cadeia para derivadas
- Atualização de pesos proporcional ao erro

### ✅ Generalização
- Dataset de treinamento separado de validação
- Rede toma decisões em cenários não treinados
- Métricas de erro em dados não vistos

### ✅ Otimização
- Momentum para evitar mínimos locais
- Taxa de aprendizado ajustável
- Critério de parada baseado em performance

---

## 📝 Personalização

### Alterar Arquitetura

Edite `src/train_network.cpp`:

```cpp
// Adicionar mais neurônios ocultos
network.addHiddenLayer(10, std::make_shared<SigmoidActivation>());

// Adicionar segunda camada oculta
network.addHiddenLayer(5, std::make_shared<SigmoidActivation>());
network.addHiddenLayer(3, std::make_shared<SigmoidActivation>());

// Usar ReLU ao invés de Sigmoid
network.addHiddenLayer(5, std::make_shared<ReLUActivation>());
```

### Alterar Hiperparâmetros

```cpp
// Criar rede com novos valores
NeuralNetwork network(4, 1, 
    0.1,   // learning rate (era 0.3)
    0.95   // momentum (era 0.9)
);
```

### Adicionar Padrões de Treinamento

Edite `createFullTrainingDataset()` em `train_network.cpp`:

```cpp
inputs.push_back({1, 0, 1, 1});  // Novo padrão
targets.push_back({0.65});        // Ação: FRENTE
```

---

## 🔗 Links Úteis

- **ARIA Docs**: http://robots.mobilerobots.com/wiki/ARIA
- **MobileSim**: http://robots.mobilerobots.com/wiki/MobileSim
- **Neural Networks**: http://neuralnetworksanddeeplearning.com/
- **Backpropagation**: https://brilliant.org/wiki/backpropagation/

---

## 💡 Dicas para Nota Máxima

1. **Commits regulares**: Faça commits pequenos e frequentes
2. **Comentários claros**: Documente o "porquê", não apenas o "o quê"
3. **Testes diversos**: Teste em múltiplos mapas do MobileSim
4. **Apresentação técnica**: Explique arquitetura, não só demonstre
5. **Métricas**: Mostre gráficos de erro, estatísticas de decisão
6. **Comparação**: Compare com versão heurística (Colisionavoidancethread.cpp)

---

## 🏆 Bônus: Algoritmos Genéticos (+20 pontos)

Para implementar AG e ganhar pontos extras:

### Estrutura Sugerida

```cpp
class GeneticOptimizer {
    // População de redes neurais
    std::vector<NeuralNetwork> population;
    
    // Avaliar fitness de cada indivíduo
    double evaluate(NeuralNetwork& network);
    
    // Seleção dos melhores
    void selection();
    
    // Crossover entre pais
    NeuralNetwork crossover(NeuralNetwork& parent1, NeuralNetwork& parent2);
    
    // Mutação aleatória
    void mutate(NeuralNetwork& network);
};
```

### Aplicações

1. **Otimizar hiperparâmetros**: learning rate, momentum
2. **Evoluir arquitetura**: número de camadas/neurônios
3. **Treinar pesos**: alternativa ao backpropagation

---

**Boa sorte na apresentação! 🎉**

*Lembre-se: o código deve rodar perfeitamente na hora!*
