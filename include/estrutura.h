#ifndef ESTRUTURA_H
#define ESTRUTURA_H

#define LINHAS 15
#define COLUNAS 30
#define TAMANHO_CELULA 40

#define LARGURA_JOGO (COLUNAS * TAMANHO_CELULA)
#define ALTURA_JOGO (LINHAS * TAMANHO_CELULA + 60)

#define TAMANHO_SPRITE 20
#define QUADROS_GELO 4
#define QUADROS_PEIXE 4

#define PAREDE_NORMAL '#'
#define PAREDE_GELO 'G'
#define CAMINHO ' '

#define MAX_FRUTAS 27
#define MAX_INIMIGOS 5
#define MAX_ROTA 24
#define NUM_OPCOES_MENU 5

#define VELOCIDADE_INIMIGO 12
#define VELOCIDADE_PERSEGUIDOR 24
#define MEMORIA_MOVIMENTO 6

// posição (linha, coluna) de uma célula do mapa
typedef struct {
    int linha;
    int coluna;
} Posicao;

// declara a base das fases
typedef struct {
    int numero;
    int totalFrutas;
    int frutasPrimeiraLeva;
    int frutasSegundaLeva;
    int frutasTerceiraLeva;
    int mapa;
    float velocidadeInimigos;
    float velocidadePerseguidores;
    int quantidadePerseguidores;
} Fase;

// declara o jogador
typedef struct {
    int linha;
    int coluna;
    int direcaoLinha;
    int direcaoColuna;
} Jogador;

// declara o inimigo
typedef struct {
    Posicao posicao;
    Posicao posicaoAnterior;
    Posicao rota[MAX_ROTA];
    int tamanhoRota;
    int indiceRota;
    int contador;
    int perseguidor;
    int sentido; // 1 = avança na rota; -1 = volta
    int velocidade;
    int ativo;

    Posicao historico[MEMORIA_MOVIMENTO];
    int quantidadeHistorico;
    int indiceHistorico;
} Inimigo;

// Telas possíveis do jogo
typedef enum {
    MENU,
    JOGANDO,
    FASE_CONCLUIDA,
    GAME_OVER,
    FIM,
    RECORDES,
    AJUSTES,
    PAUSADO,
    INSTRUCOES,
    SAIR,
} Estado;

// Situação geral do jogo
typedef struct {
    Estado estado;
    int pontos;
    int fase;
} Jogo;

// Recorde: cada nó é alocado com malloc (lista encadeada)
typedef struct Recorde {
    int pontos;
    int fase;
    struct Recorde *prox;
} Recorde;

extern Fase fases[];
extern Jogo jogo;

const Fase *obterFase(int numero);

#endif
