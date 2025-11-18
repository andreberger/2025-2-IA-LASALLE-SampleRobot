#ifndef ACTIVATIONFUNCTION_H
#define ACTIVATIONFUNCTION_H

#include <cmath>
#include <string>

/**
 * @brief Classe base abstrata para funções de ativação
 * 
 * Esta classe define a interface para todas as funções de ativação
 * utilizadas nas camadas da rede neural. Cada função de ativação
 * deve implementar tanto a função forward quanto sua derivada.
 */
class ActivationFunction {
public:
    virtual ~ActivationFunction() = default;
    
    /**
     * @brief Aplica a função de ativação ao valor de entrada
     * @param x Valor de entrada
     * @return Valor após aplicação da função de ativação
     */
    virtual double activate(double x) const = 0;
    
    /**
     * @brief Calcula a derivada da função de ativação
     * @param x Valor de entrada (ou saída da ativação, dependendo da função)
     * @return Derivada da função de ativação
     */
    virtual double derivative(double x) const = 0;
    
    /**
     * @brief Retorna o nome da função de ativação
     * @return Nome da função de ativação
     */
    virtual std::string getName() const = 0;
};

/**
 * @brief Função de ativação Sigmoide
 * 
 * Função logística que mapeia valores para o intervalo (0, 1)
 * f(x) = 1 / (1 + e^(-x))
 * 
 * Útil para camadas de saída em problemas de classificação binária
 * e para normalização de valores em redes neurais clássicas.
 */
class SigmoidActivation : public ActivationFunction {
public:
    double activate(double x) const override {
        return 1.0 / (1.0 + std::exp(-x));
    }
    
    double derivative(double x) const override {
        // Derivada: f'(x) = f(x) * (1 - f(x))
        // Assume que x é a saída da sigmoid
        return x * (1.0 - x);
    }
    
    std::string getName() const override {
        return "Sigmoid";
    }
};

/**
 * @brief Função de ativação Tangente Hiperbólica (Tanh)
 * 
 * Mapeia valores para o intervalo (-1, 1)
 * f(x) = tanh(x) = (e^x - e^(-x)) / (e^x + e^(-x))
 * 
 * Similar à sigmoid, mas centrada em zero, o que pode ajudar
 * no treinamento ao ter gradientes mais fortes.
 */
class TanhActivation : public ActivationFunction {
public:
    double activate(double x) const override {
        return std::tanh(x);
    }
    
    double derivative(double x) const override {
        // Derivada: f'(x) = 1 - f(x)^2
        // Assume que x é a saída da tanh
        return 1.0 - (x * x);
    }
    
    std::string getName() const override {
        return "Tanh";
    }
};

/**
 * @brief Função de ativação ReLU (Rectified Linear Unit)
 * 
 * f(x) = max(0, x)
 * 
 * Popular em redes profundas por não sofrer de vanishing gradient
 * e ser computacionalmente eficiente.
 */
class ReLUActivation : public ActivationFunction {
public:
    double activate(double x) const override {
        return std::max(0.0, x);
    }
    
    double derivative(double x) const override {
        // Derivada: f'(x) = 1 se x > 0, 0 caso contrário
        return x > 0.0 ? 1.0 : 0.0;
    }
    
    std::string getName() const override {
        return "ReLU";
    }
};

/**
 * @brief Função de ativação Linear (Identidade)
 * 
 * f(x) = x
 * 
 * Útil para camadas de saída em problemas de regressão
 * onde queremos valores sem transformação.
 */
class LinearActivation : public ActivationFunction {
public:
    double activate(double x) const override {
        return x;
    }
    
    double derivative(double x) const override {
        return 1.0;
    }
    
    std::string getName() const override {
        return "Linear";
    }
};

#endif // ACTIVATIONFUNCTION_H
