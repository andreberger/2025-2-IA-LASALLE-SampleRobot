#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define PadroesValidacao 4
#define PadroesTreinamento 16 // 16
#define NodosEntrada 4
#define TaxaAprendizado 0.3 //0.3 converge super rápido e com uma boa precisão.
#define Momentum 0.9 // Dificulta a convergencia da rede em minimos locais, fazendo com que convirja apenas quando realmente se tratar de um valor realmente significante.
#define MaximoPesoInicial 0.5
#define Sucesso 0.004		 // 0.0004
#define NumeroCiclos 100000 // Exibir o progresso do treinamento a cada NumeroCiclos ciclos
#define NEAR 20				 // Abaixo disso, o obstaculo esta muito proximo.


#define OUT_DIREITA        0.53   // Direita
#define OUT_ESQUERDA       0.59   // Esquerda
#define OUT_FRENTE         0.65   // Frente
#define OUT_ATRAS          0.71   // Atrás
#define OUT_PARAR          0.77   // Parar


int i, j, p, q, r; // Contadores
int IntervaloTreinamentosPrintTela;
int IndiceRandom[PadroesTreinamento];
long CiclosDeTreinamento;
float Rando;
float Error;
float AcumulaPeso;

int esquerda = 0;
int direita = 0;
int frente = 0;
int atras = 0;

int esquerdaOri = 0;
int direitaOri = 0;
int frenteOri = 0;
int atrasOri = 0;

// Estrutura da rede neural

// Camada de entrada
#define NodosEntrada 4

// Camada oculta
#define NodosOcultos 5 // 5
float Oculto[NodosOcultos];
float PesosCamadaOculta[NodosEntrada + 1][NodosOcultos];
float OcultoDelta[NodosOcultos];
float AlteracaoPesosOcultos[NodosEntrada + 1][NodosOcultos];

// Camada de saída
#define NodosSaida 1
float Saida[NodosSaida];
float SaidaDelta[NodosSaida];
float PesosSaida[NodosOcultos + 1][NodosSaida];
float AlterarPesosSaida[NodosOcultos + 1][NodosSaida];

/// Estrutura da rede neural --

float ValoresSensores[1][NodosEntrada] = {
	 {1, 0, 0, 0}};

/*
 0 = Near (obstaculo proximo) = obstruido
 1 = Far (obstaculo distante) = livre

 
	Estado		Intervalo		Centro sugerido
	Direita		0.50 – 0.56			0.53
	Esquerda		0.56 – 0.62			0.59
	Frente		0.62 – 0.68			0.65
	Atrás			0.68 – 0.74			0.71
	Parar			0.74 – 0.80			0.77S
*/

/*
Direita 		Esquerda 	Frente 	Atras 	Saida
   // 1 direcao livre
	1 				0 				0 			0 		0.5
	0 				1 				0 			0 		0.6
	0 				0 				1 			0 		0.7
	0 				0 				0 			1 		0.8
   // 2 direcoes livres
	0 				0 				1 			1 		0.7
	1 				1 				0 			0 		0.5
	0 				1 				1 			0 		0.7
	1 				0 				0 			1 		0.5
	1 				0 				1 			0 		0.7
	0 				1 				0 			1 		0.5
	// 3 direcoes livres
	0 				1 				1 			1 		0.7
	1 				0 				1 			1 		0.7
	1 				1 				0 			1 		0.5
	1 				1 				1 			0 		0.7
	1 				1 				1 			1 		0.7
	// Nenhuma direcao livre
	0 				0 				0 			0 		1.0
*/


// Direita(0.53)  Esquerda(0.59)  Frente(0.65)    ATRAS(0.71)   PARADO(0.77)
const float Input[PadroesTreinamento][NodosEntrada] = {
	// 1 direcao livre
// {D, E, F, A}	1 = livre, 0 = obstruido
	{1, 0, 0, 0}, // Caminho livre apenas a direita
	{0, 1, 0, 0}, // Caminho livre apenas a esquerda
	{0, 0, 1, 0}, // Caminho livre apenas a frente
	{0, 0, 0, 1}, // Caminho livre apenas atras
	// 2 direcoes livres
	{0, 0, 1, 1},
	{1, 1, 0, 0},
	{0, 1, 1, 0},
	{1, 0, 0, 1},
	{1, 0, 1, 0},
	{0, 1, 0, 1},
	 // 3 direcoes livres
	{0, 1, 1, 1},
	{1, 0, 1, 1},
	{1, 1, 0, 1},
	{1, 1, 1, 0},
	{1, 1, 1, 1},
	// Nenhuma direcao livre
	{0, 0, 0, 0}
};

const float Objetivo[PadroesTreinamento][NodosSaida] = {
    // 1 direcao livre
    {OUT_DIREITA}, 
    {OUT_ESQUERDA}, 
    {OUT_FRENTE}, 
    {OUT_ATRAS}, 
    // 2 direcoes livres
    {OUT_FRENTE}, 
    {OUT_DIREITA}, 
    {OUT_FRENTE}, 
    {OUT_DIREITA}, 
    {OUT_FRENTE}, 
    {OUT_ESQUERDA}, 
    // 3 direcoes livres
    {OUT_FRENTE}, 
    {OUT_FRENTE}, 
    {OUT_DIREITA}, 
    {OUT_FRENTE}, 
    {OUT_FRENTE}, 
    // Nenhuma direcao livre
    {OUT_PARAR} 
};

//Dados de validação
const float InputValidacao[PadroesValidacao][NodosEntrada] = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1}
};

const float ObjetivoValidacao[PadroesValidacao][NodosSaida] = {
    {OUT_DIREITA},
    {OUT_ESQUERDA},
    {OUT_FRENTE},
    {OUT_ATRAS}
};

//--

// Prototipos
void treinarRedeNeural();
void inicializacaoPesos();
int treinoInicialRede();
void PrintarValores();
void testarValor();
void loop(int sensorD, int sensorE, int sensorF, int sensorT);
void adaptarValoresEntrada(int sensorD, int sensorE, int sensorF, int sensorT);
void validarRedeNeural();

void validarRedeNeural() {
    float ErrorValidacao = 0.0;
    printf("\n--- Validação ---\n");

    for (int p = 0; p < PadroesValidacao; p++) {
        for (i = 0; i < NodosOcultos; i++) {
            AcumulaPeso = PesosCamadaOculta[NodosEntrada][i];
            for (j = 0; j < NodosEntrada; j++) {
                AcumulaPeso += InputValidacao[p][j] * PesosCamadaOculta[j][i];
            }
            Oculto[i] = 1.0 / (1.0 + exp(-AcumulaPeso));
        }
        for (i = 0; i < NodosSaida; i++) {
            AcumulaPeso = PesosSaida[NodosOcultos][i];
            for (j = 0; j < NodosOcultos; j++) {
                AcumulaPeso += Oculto[j] * PesosSaida[j][i];
            }
            Saida[i] = 1.0 / (1.0 + exp(-AcumulaPeso));
            ErrorValidacao += 0.5 * (ObjetivoValidacao[p][i] - Saida[i]) * (ObjetivoValidacao[p][i] - Saida[i]);
        }
        printf("Padrao %d | Objetivo: %.2f | Saida: %.2f\n", p, ObjetivoValidacao[p][0], Saida[0]);
    }

    printf("Erro de validacao: %f\n", ErrorValidacao);
}

void adaptarValoresEntrada(int sensorD, int sensorE, int sensorF, int sensorT) // Converter os valores dos sensores para binário.
{
	// Direita
	direitaOri = sensorD;
	if (direitaOri > NEAR)
		direita = 1;
	else
		direita = 0;

	// Esquerda
	esquerdaOri = sensorE;
	if (esquerdaOri > NEAR)
		esquerda = 1;
	else
		esquerda = 0;

	// Frente
	frenteOri = sensorF;
	if (frenteOri > NEAR)
		frente = 1;
	else
		frente = 0;

	// Atras
	atrasOri = sensorT;
	if (atrasOri > NEAR)
		atras = 1;
	else
		atras = 0;
}

void treinarRedeNeural()
{
	int numeroTreinamentosExecutados = 0;
	int p = 0;

	for (p = 0; p < PadroesTreinamento; p++)
		IndiceRandom[p] = p;

	numeroTreinamentosExecutados = treinoInicialRede(); // Efetua o treinamento
	if (numeroTreinamentosExecutados > 0)					 // Se "numeroTreinamentosExecutados" for maior do que zero, indica que a rede executou "numeroTreinamentosExecutados" treinamentos, para aprender o padrão.
		printf("\n\n\nA rede foi treinada com sucesso, apos %d trinamentos", numeroTreinamentosExecutados);
}

void inicializacaoPesos()
{
	// Inicializa os valores dos pesos das camadas ocultas e da camada de entrada
	for (i = 0; i < NodosOcultos; i++)
	{
		for (j = 0; j <= NodosEntrada; j++)
		{
			AlteracaoPesosOcultos[j][i] = 0.0;
			Rando = (rand() % 100) / 100;
			PesosCamadaOculta[j][i] = 2.0 * (Rando - 0.5) * MaximoPesoInicial;
		}
	}

	// Inicializa os pesos da camada de saida
	for (i = 0; i < NodosSaida; i++)
	{
		for (j = 0; j <= NodosOcultos; j++)
		{
			AlterarPesosSaida[j][i] = 0.0;
			Rando = (rand() % 100) / 100;
			PesosSaida[j][i] = 2.0 * (Rando - 0.5) * MaximoPesoInicial;
		}
	}
	printf("Initial/Untrained Saidas: \n\n");
	PrintarValores();
}

int treinoInicialRede()
{
	IntervaloTreinamentosPrintTela = NumeroCiclos;
	inicializacaoPesos();

	// Inicio do treinamento
	for (CiclosDeTreinamento = 1; CiclosDeTreinamento < 2147483647; CiclosDeTreinamento++)
	{
		// Randomizar a ordem dos treinamentos, evitando que caia nas regioes de minimos comuns
		for (p = 0; p < PadroesTreinamento; p++)
		{
			q = rand() % PadroesTreinamento;
			;
			r = IndiceRandom[p];
			IndiceRandom[p] = IndiceRandom[q];
			IndiceRandom[q] = r;
		}
		Error = 0.0;
		// Navega entre todos os padroes de treinamento
		for (q = 0; q < PadroesTreinamento; q++)
		{
			p = IndiceRandom[q];

			// Processa a ativacoes da(s) camada(s) oculta(s) - Por hora ha apenas uma camada oculta
			for (i = 0; i < NodosOcultos; i++)
			{
				AcumulaPeso = PesosCamadaOculta[NodosEntrada][i];
				for (j = 0; j < NodosEntrada; j++)
				{
					AcumulaPeso += Input[p][j] * PesosCamadaOculta[j][i];
				}
				Oculto[i] = 1.0 / (1.0 + exp(-AcumulaPeso)); // Funcao Sigmoide - retorna valores no intervalo compreendido entre zero e um.
			}

			// Processa a ativacoes dos neur�nios da camada de saida e calcula o erro
			for (i = 0; i < NodosSaida; i++)
			{
				AcumulaPeso = PesosSaida[NodosOcultos][i];
				for (j = 0; j < NodosOcultos; j++)
				{
					AcumulaPeso += Oculto[j] * PesosSaida[j][i];
				}
				Saida[i] = 1.0 / (1.0 + exp(-AcumulaPeso));
				SaidaDelta[i] = (Objetivo[p][i] - Saida[i]) * Saida[i] * (1.0 - Saida[i]);
				Error += 0.5 * (Objetivo[p][i] - Saida[i]) * (Objetivo[p][i] - Saida[i]);
			}

			// Backpropagation - Propaga o erro para as camadas anteriores
			for (i = 0; i < NodosOcultos; i++)
			{
				AcumulaPeso = 0.0;
				for (j = 0; j < NodosSaida; j++)
				{
					AcumulaPeso += PesosSaida[i][j] * SaidaDelta[j];
				}
				OcultoDelta[i] = AcumulaPeso * Oculto[i] * (1.0 - Oculto[i]); // Funcao Delta/Regra Delta
			}

			// Atualiza os pesos, indo da entrada para a camada oculta
			for (i = 0; i < NodosOcultos; i++)
			{
				AlteracaoPesosOcultos[NodosEntrada][i] = TaxaAprendizado * OcultoDelta[i] + Momentum * AlteracaoPesosOcultos[NodosEntrada][i];
				PesosCamadaOculta[NodosEntrada][i] += AlteracaoPesosOcultos[NodosEntrada][i];
				for (j = 0; j < NodosEntrada; j++)
				{
					AlteracaoPesosOcultos[j][i] = TaxaAprendizado * Input[p][j] * OcultoDelta[i] + Momentum * AlteracaoPesosOcultos[j][i];
					PesosCamadaOculta[j][i] += AlteracaoPesosOcultos[j][i];
				}
			}

			// Atualiza os pesos, indo da camada oculta para a saida
			for (i = 0; i < NodosSaida; i++)
			{
				AlterarPesosSaida[NodosOcultos][i] = TaxaAprendizado * SaidaDelta[i] + Momentum * AlterarPesosSaida[NodosOcultos][i];
				PesosSaida[NodosOcultos][i] += AlterarPesosSaida[NodosOcultos][i];
				for (j = 0; j < NodosOcultos; j++)
				{
					AlterarPesosSaida[j][i] = TaxaAprendizado * Oculto[j] * SaidaDelta[i] + Momentum * AlterarPesosSaida[j][i];
					PesosSaida[j][i] += AlterarPesosSaida[j][i];
				}
			}
		}

		// Exibir na tela o estatus do treinamento
		IntervaloTreinamentosPrintTela = IntervaloTreinamentosPrintTela - 1;
		if (IntervaloTreinamentosPrintTela == 0)
		{
			printf("CiclosDeTreinamento: %ld  Error: %f \n\n", CiclosDeTreinamento, Error);
			PrintarValores();

			if (CiclosDeTreinamento == 1)
				IntervaloTreinamentosPrintTela = NumeroCiclos - 1;
			else
				IntervaloTreinamentosPrintTela = NumeroCiclos;
		}

		// Encerra o treinamento caso o erro seja menor do que o erro considerado aceitavel (configurado)
		if (Error < Sucesso)
			return CiclosDeTreinamento;
	}

	printf("CiclosDeTreinamento: %ld  Error: %f \n\n", CiclosDeTreinamento, Error);
	PrintarValores();
	printf("Treinamento concluido! \n\n");
	IntervaloTreinamentosPrintTela = 1;
}

void PrintarValores()
{
	for (p = 0; p < PadroesTreinamento; p++)
	{
		printf("  Padrao de treinamento: %d \n", p);
		printf("  Entrada: ");
		for (i = 0; i < NodosEntrada; i++)
			printf(" %f  ", Input[p][i]);

		printf("\n  Objetivo: ");
		for (i = 0; i < NodosSaida; i++)
			printf(" %f ", Objetivo[p][i]);

		// Computar as ativacoes da camada oculta
		for (i = 0; i < NodosOcultos; i++)
		{
			AcumulaPeso = PesosCamadaOculta[NodosEntrada][i];
			for (j = 0; j < NodosEntrada; j++)
			{
				AcumulaPeso += Input[p][j] * PesosCamadaOculta[j][i];
			}
			Oculto[i] = 1.0 / (1.0 + exp(-AcumulaPeso)); // Funcao Sigmoide
		}

		// Computar as ativacoes da camada de saida e calcular os seus erros
		for (i = 0; i < NodosSaida; i++)
		{
			AcumulaPeso = PesosSaida[NodosOcultos][i];
			for (j = 0; j < NodosOcultos; j++)
			{
				AcumulaPeso += Oculto[j] * PesosSaida[j][i];
			}
			Saida[i] = 1.0 / (1.0 + exp(-AcumulaPeso)); // Funcao Sigmoide
		}

		printf("  Saida: ");
		for (i = 0; i < NodosSaida; i++)
			printf(" %f \n", Saida[i]);
	}
}

void testarValor()
{
	printf("  Testando valores: \n");
	printf("  Training Pattern: \n");
	printf("  Input \n");
	for (i = 0; i < NodosEntrada; i++)
		printf(" %f ", ValoresSensores[0][i]);

	printf("\n\n Direita,            Esquerda,              Frente,            Atras \n\n");

	for (p = 0; p < 1; p++)
	{
		// Computar as ativacoes da camada oculta
		for (i = 0; i < NodosOcultos; i++)
		{
			AcumulaPeso = PesosCamadaOculta[NodosEntrada][i];
			for (j = 0; j < NodosEntrada; j++)
			{
				AcumulaPeso += ValoresSensores[p][j] * PesosCamadaOculta[j][i];
			}
			Oculto[i] = 1.0 / (1.0 + exp(-AcumulaPeso)); // Funcao Sigmoide
		}
		// Computar as ativacoes da camada de saida e calcular os seus erros
		for (i = 0; i < NodosSaida; i++)
		{
			AcumulaPeso = PesosSaida[NodosOcultos][i];
			for (j = 0; j < NodosOcultos; j++)
			{
				AcumulaPeso += Oculto[j] * PesosSaida[j][i];
			}
			Saida[i] = 1.0 / (1.0 + exp(-AcumulaPeso)); // Funcao Sigmoide
		}

		printf("  Saida \n");
		for (i = 0; i < NodosSaida; i++)
			printf(" %f ", Saida[i]);

		printf("\n\n");

		if (Saida[0] >= 0.50 && Saida[0] < 0.56)
			printf("DIREITA\n\n");
		else if (Saida[0] >= 0.56 && Saida[0] < 0.62)
			printf("ESQUERDA\n\n");
		else if (Saida[0] >= 0.62 && Saida[0] < 0.68)
			printf("FRENTE\n\n");
		else if (Saida[0] >= 0.68 && Saida[0] < 0.74)
			printf("ATRAS\n\n");
		else if (Saida[0] >= 0.74 && Saida[0] < 0.80)
			printf("PARAR\n\n");
		else
			printf("POUSAR\n\n");

		printf(" %f ", Saida[0]);
	}
}

void loop(int sensorD, int sensorE, int sensorF, int sensorT) // Loop principal do Arduino
{
	adaptarValoresEntrada(sensorD, sensorE, sensorF, sensorT);
	ValoresSensores[0][0] = direita;
	ValoresSensores[0][1] = esquerda;
	ValoresSensores[0][2] = frente;
	ValoresSensores[0][3] = atras;

	printf(" Direita: %d  Direita Original: %d \n", direita, direitaOri);
	printf(" Esquerda: %d  Esquerda Original: %d \n", esquerda, esquerdaOri);
	printf(" Frente: %d  Frente Original: %d \n", frente, frenteOri);
	printf(" Atras: %d  Atras Original: %d \n", atras, atrasOri);

	testarValor();
}

int main(void)
{
	srand(time(NULL));
	int sensorD, sensorE, sensorF, sensorT;
	treinarRedeNeural();
	validarRedeNeural();

	/*while(1)
	{
	  printf("Informe os valores dos sensores D, E, F e T, respectivamente:");
	  scanf("%d %d %d %d", &sensorD, &sensorE, &sensorF, &sensorT);
	  printf("\n\nDireita %d Esquerda %d Frente %d Atr�s %d\n\n", sensorD, sensorE, sensorF, sensorT);
	  loop(sensorD, sensorE, sensorF, sensorT);
	}*/

	printf("Informe os valores dos sensores D, E, F e T, respectivamente:\n\n");
	/*loop(100, 10, 10, 10); // Direita seria logico
	loop(10, 100, 10, 10); // Esquerda seria logico
	loop(10, 10, 100, 10); // Frete seria logico
	loop(10, 10, 10, 100); // Atras seria logico

	loop(100, 100, 100, 100); // Qualquer direcao seria logico (mas vamos seguir em frente, ou então simplesmente não interferir no movimento do robô)
	loop(10, 10, 10, 10);	  // Parar seria logico

	loop(100, 100, 10, 10); // Direita ou esquerda seria logico
	loop(10, 10, 100, 100); // Frente ou atras seria logico*/

	return 0;
}
