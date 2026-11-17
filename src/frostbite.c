#include <stdlib.h>
#include <stdio.h>
#include "raylib.h"

void preencherMapaBase(void);
void abrirCorredor(int linha, int inicio, int fim);

#define LINHAS 15
#define COLUNAS 30
#define TAMANHO_CELULA 40

// Resolução lógica do jogo. Tudo é desenhado numa textura desse tamanho
// e depois escalado para a janela (assim funciona em qualquer resolução).
#define LARGURA_JOGO (COLUNAS * TAMANHO_CELULA)
#define ALTURA_JOGO (LINHAS * TAMANHO_CELULA + 60)

// Sprites em pixel art: arte de 20x20 desenhada em 2x (40px por célula).
#define TAMANHO_SPRITE 20
#define QUADROS_GELO 4
#define QUADROS_PEIXE 4

#define PAREDE_NORMAL '#'
#define PAREDE_GELO 'G'
#define CAMINHO ' '

#define MAX_FRUTAS 27
#define MAX_INIMIGOS 5
#define MAX_ROTA 24
#define NUM_OPCOES_MENU 4

#define VELOCIDADE_INIMIGO 12
#define VELOCIDADE_PERSEGUIDOR 24
#define MEMORIA_MOVIMENTO 6

//declara a posicao do jogador
typedef struct {
    int linha;
    int coluna;
} Posicao;

//declara a base das fases
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
    int nivelInteligencia;
} Fase;

Fase fases[] = {
    {1, 9, 9, 0, 0, 1, 12, 24, 1, 1},
    {2, 22, 13, 9, 0, 2, 10, 20, 1, 2},
    {3, 27, 9, 13, 5, 3, 8, 16, 2, 3}
};

//declara o jogador
typedef struct {
    int linha;
    int coluna;
    int direcaoLinha;
    int direcaoColuna;
} Jogador;

//declara o inimigo
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
    SAIR,
} Estado;

// Situação geral do jogo
typedef struct {
    Estado estado;
    int pontos;
    int fase;
} Jogo;

// Recorde: cada nó é alocado com malloc (lista encadeada)
typedef struct Recorde{
    int pontos;
    int fase;
    struct Recorde *prox;
}Recorde;

Recorde *recordes = NULL;

#define ARQUIVO_RECORDE "recordes.txt"
// Textura onde o jogo é desenhado na resolução lógica (LARGURA_JOGO x ALTURA_JOGO).
// No fim do frame ela é desenhada na janela, escalada e centralizada.
RenderTexture2D alvo;

// Sprites do design (carregados em main, liberados no fim).
Texture2D spriteChao;
Texture2D spriteParede;
Texture2D spriteGelo;
Texture2D spritePinguim;
Texture2D spriteFoca;
Texture2D spriteUrso;
Texture2D spritePeixe;
Texture2D spriteFundo;
Texture2D logoMenu;

char mapa[LINHAS][COLUNAS];
Jogador jogador = {1, 1, 0, 1};

Posicao frutas[MAX_FRUTAS] = {
    // Primeira leva: 13 frutas
    {1, 5}, {2, 12}, {4, 3},
    {4, 17}, {6, 24}, {8, 10},
    {10, 5}, {12, 19}, {13, 26},
    {2, 20}, {6, 14}, {2, 16},
    {11, 7},

    // Segunda leva: 9 frutas novas
    {1, 10}, {3, 25}, {5, 6},
    {7, 22}, {9, 3}, {10, 17},
    {11, 25}, {13, 15}, {12, 5},

    // Terceira leva: 5 frutas novas
    {2, 8},
    {2, 24},
    {8, 5},
    {11, 18},
    {13, 23}
};

int frutaColetada[MAX_FRUTAS] = {0};
int indiceInicialFrutas = 0;
int quantidadeFrutasAtivas = 9;
int segundaLevaFase2 = 0;
int terceiraLevaFase3 = 0;

Inimigo inimigos[MAX_INIMIGOS];
Jogo jogo;
int opcaoMenu = 0;
double tempoUltimoMovimento = 0.0;
float volumeMusica = 0.5f;

//manipulação das fases
const Fase *obterFase(int numero) {

    int quantidadeFases = sizeof(fases) / sizeof(fases[0]);

    for (int i = 0; i < quantidadeFases; i++) {

        if (fases[i].numero == numero) {
            return &fases[i];
        }
    }
    return NULL;
}

void inicializarMapaFase3(void) {
    preencherMapaBase();

    // Paredes normais para criar obstáculos
    mapa[3][6] = PAREDE_NORMAL;
    mapa[3][15] = PAREDE_NORMAL;
    mapa[3][24] = PAREDE_NORMAL;

    mapa[6][10] = PAREDE_NORMAL;
    mapa[6][20] = PAREDE_NORMAL;

    mapa[9][5] = PAREDE_NORMAL;
    mapa[9][15] = PAREDE_NORMAL;
    mapa[9][25] = PAREDE_NORMAL;

    mapa[12][10] = PAREDE_NORMAL;
    mapa[12][20] = PAREDE_NORMAL;

    // Corredores horizontais
    abrirCorredor(2, 2, 27);
    abrirCorredor(5, 2, 27);
    abrirCorredor(8, 2, 27);
    abrirCorredor(11, 2, 27);
    abrirCorredor(13, 2, 27);

    // Corredores verticais
    for (int l = 2; l <= 13; l++) {
        if (mapa[l][3] != PAREDE_NORMAL)
            mapa[l][3] = CAMINHO;

        if (mapa[l][14] != PAREDE_NORMAL)
            mapa[l][14] = CAMINHO;

        if (mapa[l][26] != PAREDE_NORMAL)
            mapa[l][26] = CAMINHO;
    }

    // Área inicial do jogador
    mapa[1][1] = CAMINHO;
    mapa[1][2] = CAMINHO;
    mapa[1][3] = CAMINHO;
}

int calcularDistancia(Posicao a, Posicao b) {
    int diferencaLinha = abs(a.linha - b.linha);
    int diferencaColuna = abs(a.coluna - b.coluna);

    return diferencaLinha + diferencaColuna;
}

// Cria uma parede horizontal com uma abertura.
void paredeHorizontal(int linha, int inicio, int fim, int abertura) {
    for (int c = inicio; c <= fim; c++) {
        if (c != abertura) {
            mapa[linha][c] = PAREDE_NORMAL;
        }
    }
}


void abrirCorredor(int linha, int inicio, int fim) {
    for (int c = inicio; c <= fim; c++) {
        if (mapa[linha][c] != PAREDE_NORMAL) {
            mapa[linha][c] = CAMINHO;
        }
    }
}
void preencherMapaBase(void)
{
    for (int l = 0; l < LINHAS; l++) {
        for (int c = 0; c < COLUNAS; c++) {
            if (l == 0 || l == LINHAS - 1 ||
                c == 0 || c == COLUNAS - 1) {
                mapa[l][c] = PAREDE_NORMAL;
            } else {
                mapa[l][c] = PAREDE_GELO;
            }
        }
    }
}

void inicializarMapaFase1(void) {
    preencherMapaBase();

    // Paredes permanentes distribuídas pelo labirinto.
    mapa[3][4] = PAREDE_NORMAL;
    mapa[3][15] = PAREDE_NORMAL;
    mapa[5][9] = PAREDE_NORMAL;
    mapa[6][22] = PAREDE_NORMAL;
    mapa[8][4] = PAREDE_NORMAL;
    mapa[8][17] = PAREDE_NORMAL;
    mapa[10][12] = PAREDE_NORMAL;
    mapa[12][23] = PAREDE_NORMAL;

    // Área inicial do jogador.
    mapa[1][1] = CAMINHO;
    mapa[1][2] = CAMINHO;

    // Alguns corredores e pequenas áreas abertas.
    abrirCorredor(2, 2, 8);
    abrirCorredor(4, 5, 7);
    abrirCorredor(6, 2, 5);
    abrirCorredor(8, 20, 26);
    abrirCorredor(10, 5, 8);
    abrirCorredor(13, 4, 12);
    abrirCorredor(13, 19, 26);

    // Área central parcialmente aberta.
    for (int l = 7; l <= 9; l++) {
        for (int c = 13; c <= 15; c++) {
            if (mapa[l][c] != PAREDE_NORMAL) {
                mapa[l][c] = CAMINHO;
            }
        }
    }
}

void inicializarMapaFase2(void)
{
    preencherMapaBase();

    // Paredes permanentes da fase 2.
    mapa[2][6] = PAREDE_NORMAL;
    mapa[2][18] = PAREDE_NORMAL;
    mapa[4][11] = PAREDE_NORMAL;
    mapa[4][23] = PAREDE_NORMAL;
    mapa[6][7] = PAREDE_NORMAL;
    mapa[7][17] = PAREDE_NORMAL;
    mapa[9][10] = PAREDE_NORMAL;
    mapa[10][21] = PAREDE_NORMAL;
    mapa[12][14] = PAREDE_NORMAL;

    // Corredores horizontais.
    abrirCorredor(2, 2, 27);
    abrirCorredor(4, 2, 27);
    abrirCorredor(7, 2, 27);
    abrirCorredor(10, 2, 27);
    abrirCorredor(12, 2, 27);

    // Corredores verticais para conectar as regiões.
    for (int l = 2; l <= 12; l++) {
        if (mapa[l][3] != PAREDE_NORMAL) {
            mapa[l][3] = CAMINHO;
        }

        if (mapa[l][14] != PAREDE_NORMAL) {
            mapa[l][14] = CAMINHO;
        }

        if (mapa[l][26] != PAREDE_NORMAL) {
            mapa[l][26] = CAMINHO;
        }
    }

    // Área inicial do jogador.
    mapa[1][1] = CAMINHO;
    mapa[1][2] = CAMINHO;
    mapa[1][3] = CAMINHO;
}

int posicaoValida(int linha, int coluna) {
    return linha >= 0 && linha < LINHAS &&
           coluna >= 0 && coluna < COLUNAS;
}

int existeFruta(int linha, int coluna) {
    for (int i = 0; i < MAX_FRUTAS; i++) {
        if (!frutaColetada[i] &&
            frutas[i].linha == linha &&
            frutas[i].coluna == coluna) {
            return 1;
        }
    }

    return 0;
}

int existeInimigo(int linha, int coluna) {
    for (int i = 0; i < MAX_INIMIGOS; i++) {
        if (!inimigos[i].ativo) {
            continue;
        }
        if (inimigos[i].posicao.linha == linha &&
            inimigos[i].posicao.coluna == coluna) {
            return 1;
        }
    }

    return 0;
}

void coletarFruta(void) {
    for (int i = indiceInicialFrutas;
         i < indiceInicialFrutas + quantidadeFrutasAtivas;
         i++) {

        if (!frutaColetada[i] &&
            frutas[i].linha == jogador.linha &&
            frutas[i].coluna == jogador.coluna) {

            frutaColetada[i] = 1;
            jogo.pontos += 10;
        }
    }
}

int contarFrutasColetadas(void) {
    int total = 0;

    for (int i = indiceInicialFrutas;
         i < indiceInicialFrutas + quantidadeFrutasAtivas;
         i++) {

        if (frutaColetada[i]) {
            total++;
        }
    }

    return total;
}

void moverJogador(int direcaoLinha, int direcaoColuna) {
    // Guardamos a direção mesmo quando existe uma parede.
    jogador.direcaoLinha = direcaoLinha;
    jogador.direcaoColuna = direcaoColuna;

    int novaLinha = jogador.linha + direcaoLinha;
    int novaColuna = jogador.coluna + direcaoColuna;

    if (!posicaoValida(novaLinha, novaColuna)) {
        return;
    }

    if (mapa[novaLinha][novaColuna] != CAMINHO) {
        return;
    }


    jogador.linha = novaLinha;
    jogador.coluna = novaColuna;

    coletarFruta();
}

void quebrarGelo() {
    int dl = jogador.direcaoLinha;
    int dc = jogador.direcaoColuna;

    // Não faz nada se o jogador não estiver apontando
    // para nenhuma direção.
    if (dl == 0 && dc == 0) {
        return;
    }

    int l = jogador.linha + dl;
    int c = jogador.coluna + dc;

    while (posicaoValida(l, c)) {
        if (mapa[l][c] == PAREDE_NORMAL) {
            break;
        }

        // Não atravessa inimigos.
        if (existeInimigo(l, c)) {
            break;
        }

        // Só transforma gelo em caminho.
        if (mapa[l][c] == PAREDE_GELO) {
            mapa[l][c] = CAMINHO;
        }

        l += dl;
        c += dc;
    }
}

void reconstruirGelo() {
    int dl = jogador.direcaoLinha;
    int dc = jogador.direcaoColuna;

    if (dl == 0 && dc == 0) {
        return;
    }

    int l = jogador.linha + dl;
    int c = jogador.coluna + dc;

    while (posicaoValida(l, c)) {
        if (mapa[l][c] == PAREDE_NORMAL) {
            break;
        }

        // Para antes de alcançar um inimigo.
        if (existeInimigo(l, c)) {
            break;
        }

        // Não coloca gelo sobre frutas.
        if (mapa[l][c] == CAMINHO &&
            !existeFruta(l, c)) {
            mapa[l][c] = PAREDE_GELO;
        }

        l += dl;
        c += dc;
    }
}

// Define uma rota fixa para cada inimigo.
void definirRota(int indice, const Posicao rota[], int tamanho) {
    inimigos[indice].tamanhoRota = tamanho;
    inimigos[indice].indiceRota = 0;
    inimigos[indice].contador = 0;
    inimigos[indice].sentido = 1;

    for (int i = 0; i < tamanho; i++) {
        inimigos[indice].rota[i] = rota[i];
    }

    inimigos[indice].posicao = rota[0];
    inimigos[indice].posicaoAnterior = rota[0];
    inimigos[indice].perseguidor = 0;
}

void inicializarInimigos(void) {
    for (int i = 0; i < MAX_INIMIGOS; i++) {
        inimigos[i].ativo = 0;
    }
    const Fase *config = obterFase(jogo.fase);

    if (config == NULL) {
        return;
    }
    // Inimigo 1: patrulha a região abaixo da barreira de gelo.
    Posicao rota1[] = {
        {6,2}, {6,3}, {6,4}, {6,5},
        {6,4}, {6,3}
    };

    // Inimigo 2: patrulha a parte direita.
    Posicao rota2[] = {
        {8,20}, {8,21}, {8,22}, {8,23},
        {8,24}, {8,25}, {8,26}, {8,25},
        {8,24}, {8,23}, {8,22}, {8,21}
    };

    // Inimigo 3: patrulha a região inferior.
    Posicao rota3[] = {
        {13,4}, {13,5}, {13,6}, {13,7},
        {13,8}, {13,9}, {13,10}, {13,11},
        {13,12}, {13,11}, {13,10}, {13,9},
        {13,8}, {13,7}, {13,6}, {13,5}
    };

    definirRota(0, rota1, sizeof(rota1) / sizeof(rota1[0]));
    definirRota(1, rota2, sizeof(rota2) / sizeof(rota2[0]));
    definirRota(2, rota3, sizeof(rota3) / sizeof(rota3[0]));

    inimigos[0].ativo = 1;
    inimigos[1].ativo = 1;
    inimigos[2].ativo = 1;

    for (int i = 0; i < 3; i++) {
        inimigos[i].velocidade = config->velocidadeInimigos;
    }

    inimigos[3].posicao = (Posicao){13, 20};
    inimigos[3].posicaoAnterior = inimigos[3].posicao;
    inimigos[3].velocidade = config->velocidadePerseguidores;
    mapa[13][20] = CAMINHO;
    inimigos[3].perseguidor = 1;
    inimigos[3].contador = -300;
    inimigos[3].tamanhoRota = 0;
    inimigos[3].indiceRota = 0;
    inimigos[3].ativo = 1;

    if (jogo.fase == 3) {
        inimigos[4].posicao = (Posicao){1, 25};
        inimigos[4].posicaoAnterior = inimigos[4].posicao;
        inimigos[4].velocidade = config->velocidadePerseguidores;
        inimigos[4].perseguidor = 1;
        inimigos[4].contador = -450;
        inimigos[4].tamanhoRota = 0;
        inimigos[4].indiceRota = 0;
        inimigos[4].ativo = 1;

        mapa[1][25] = CAMINHO;
    }
    // Inicializa a memória de movimento dos inimigos.
    for (int i = 0; i < MAX_INIMIGOS; i++) {
        inimigos[i].quantidadeHistorico = 0;
        inimigos[i].indiceHistorico = 0;

        if (inimigos[i].ativo) {
            inimigos[i].historico[0] = inimigos[i].posicao;
            inimigos[i].quantidadeHistorico = 1;
        }
    }
}

void prepararRotasInimigos(void) {
    for (int i = 0; i < MAX_INIMIGOS; i++) {
        if (!inimigos[i].ativo) {
            continue;
        }
        for (int j = 0; j < inimigos[i].tamanhoRota; j++) {
            int l = inimigos[i].rota[j].linha;
            int c = inimigos[i].rota[j].coluna;

            if (mapa[l][c] == PAREDE_GELO) {
                mapa[l][c] = CAMINHO;
            }
        }
    }
}

Posicao escolherMovimentoInteligente(Posicao opcoes[], int quantidade, int indiceInimigo) {
    Posicao jogadorPos = {jogador.linha, jogador.coluna};

    Posicao melhor;
    int menorDistancia = LINHAS * COLUNAS + 1;
    int encontrou = 0;

    for (int i = 0; i < quantidade; i++) {

        int ehPosicaoAnterior =
            opcoes[i].linha == inimigos[indiceInimigo].posicaoAnterior.linha &&
            opcoes[i].coluna == inimigos[indiceInimigo].posicaoAnterior.coluna;

        // Evita voltar à posição anterior se houver outra opção.
        if (quantidade > 1 && ehPosicaoAnterior) {
            continue;
        }

        int distancia = calcularDistancia(opcoes[i], jogadorPos);

        if (!encontrou || distancia < menorDistancia) {
            menorDistancia = distancia;
            melhor = opcoes[i];
            encontrou = 1;
        }
    }

    if (!encontrou) {
        return opcoes[0];
    }

    return melhor;
}

int posicaoNoHistorico(Inimigo *inimigo, Posicao posicao) {
    for (int i = 0; i < inimigo->quantidadeHistorico; i++) {
        if (inimigo->historico[i].linha == posicao.linha &&
            inimigo->historico[i].coluna == posicao.coluna) {
            return 1;
        }
    }

    return 0;
}

void moverInimigos(void) {
    int dl[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    for (int i = 0; i < MAX_INIMIGOS; i++) {
        if (!inimigos[i].ativo) {
            continue;
        }
        if (inimigos[i].perseguidor) {
            continue;
        }

        inimigos[i].contador++;

        if (inimigos[i].contador < inimigos[i].velocidade) {
            continue;
        }

        inimigos[i].contador = 0;

        Posicao opcoes[4];
        int quantidade = 0;

        for (int d = 0; d < 4; d++) {
            int nl = inimigos[i].posicao.linha + dl[d];
            int nc = inimigos[i].posicao.coluna + dc[d];

            if (!posicaoValida(nl, nc)) {
                continue;
            }

            // Não atravessa gelo nem paredes.
            if (mapa[nl][nc] != CAMINHO) {
                continue;
            }

            // Não entra na célula de outro inimigo.
            int ocupado = 0;

            for (int j = 0; j < MAX_INIMIGOS; j++) {
                if (j != i &&
                    inimigos[j].posicao.linha == nl &&
                    inimigos[j].posicao.coluna == nc) {
                    ocupado = 1;
                    break;
                }
            }

            if (ocupado) {
                continue;
            }

            opcoes[quantidade++] = (Posicao){nl, nc};
        }

        // Escolhe um movimento evitando posições visitadas recentemente.
        if (quantidade > 0) {
            Posicao opcoesNovas[4];
            int quantidadeNovas = 0;

            // Separa as posições que não estão no histórico.
            for (int j = 0; j < quantidade; j++) {
                if (!posicaoNoHistorico(&inimigos[i], opcoes[j])) {
                    opcoesNovas[quantidadeNovas++] = opcoes[j];
                }
            }

            // Se todas as opções já foram visitadas,
            // permite voltar a alguma posição para não ficar parado.
            Posicao destino;

            if (quantidadeNovas > 0) {
                int escolha = GetRandomValue(0, quantidadeNovas - 1);
                destino = opcoesNovas[escolha];
            } else {
                int escolha = GetRandomValue(0, quantidade - 1);
                destino = opcoes[escolha];
            }

            // Realiza o movimento.
            inimigos[i].posicaoAnterior = inimigos[i].posicao;
            inimigos[i].posicao = destino;

            // Registra a nova posição no histórico circular.
            inimigos[i].indiceHistorico =
                (inimigos[i].indiceHistorico + 1) % MEMORIA_MOVIMENTO;

            inimigos[i].historico[inimigos[i].indiceHistorico] = destino;

            if (inimigos[i].quantidadeHistorico < MEMORIA_MOVIMENTO) {
                inimigos[i].quantidadeHistorico++;
            }
        }
    }
}

int buscarProximoPasso(Posicao origem, Posicao destino,Posicao *proximoPasso) {
    int visitado[LINHAS][COLUNAS] = {0};
    Posicao primeiro[LINHAS][COLUNAS];
    Posicao fila[LINHAS * COLUNAS];

    int inicio = 0;
    int fim = 0;

    fila[fim++] = origem;
    visitado[origem.linha][origem.coluna] = 1;

    int dl[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    while (inicio < fim) {
        Posicao atual = fila[inicio++];

        if (atual.linha == destino.linha &&
            atual.coluna == destino.coluna) {
            *proximoPasso = primeiro[destino.linha][destino.coluna];

            if (origem.linha == destino.linha &&
                origem.coluna == destino.coluna) {
                *proximoPasso = origem;
            }

            return 1;
        }

        for (int d = 0; d < 4; d++) {
            int nl = atual.linha + dl[d];
            int nc = atual.coluna + dc[d];

            if (!posicaoValida(nl, nc) ||
                visitado[nl][nc]) {
                continue;
            }

            // O jogador é o destino; as demais células
            // precisam estar livres.
            if (mapa[nl][nc] != CAMINHO &&
                !(nl == destino.linha &&
                  nc == destino.coluna)) {
                continue;
            }

            // Não atravessa outros inimigos.
            int ocupado = 0;

            for (int i = 0; i < MAX_INIMIGOS; i++) {
                if (!inimigos[i].ativo) {
                    continue;
                }
                if (inimigos[i].posicao.linha == nl &&
                    inimigos[i].posicao.coluna == nc &&
                    !(inimigos[i].posicao.linha == origem.linha &&
                      inimigos[i].posicao.coluna == origem.coluna)) {
                    ocupado = 1;
                    break;
                }
            }

            if (ocupado) {
                continue;
            }

            Posicao vizinho = {nl, nc};

            visitado[nl][nc] = 1;

            if (atual.linha == origem.linha &&
                atual.coluna == origem.coluna) {
                primeiro[nl][nc] = vizinho;
            } else {
                primeiro[nl][nc] =
                    primeiro[atual.linha][atual.coluna];
            }

            fila[fim++] = vizinho;
        }
    }

    return 0;
}

void moverPerseguidor(void) {
    for (int i = 0; i < MAX_INIMIGOS; i++) {
        if (!inimigos[i].ativo) {
            continue;
        }
        if (!inimigos[i].perseguidor) {
            continue;
        }

        inimigos[i].contador++;

        if (inimigos[i].contador < inimigos[i].velocidade) {
        continue;
        }

        inimigos[i].contador = 0;

        Posicao origem = inimigos[i].posicao;
        Posicao destino = {jogador.linha, jogador.coluna};
        Posicao proximo;

        if (buscarProximoPasso(origem, destino, &proximo)) {
            // Não permite ocupar a posição de outro inimigo.
            if (!existeInimigo(proximo.linha, proximo.coluna) ||
                (proximo.linha == origem.linha &&
                 proximo.coluna == origem.coluna)) {
                inimigos[i].posicao = proximo;
            }
        }
    }
}

int jogadorAtingido(void) {
    for (int i = 0; i < MAX_INIMIGOS; i++) {
        if (!inimigos[i].ativo) {
            continue;
        }
        if (jogador.linha == inimigos[i].posicao.linha &&
            jogador.coluna == inimigos[i].posicao.coluna) {
            return 1;
        }
    }

    return 0;
}

// Procura um sprite em caminhos comuns, para funcionar tanto rodando
// da raiz do projeto quanto de dentro de src/.
static const char *caminhoSprite(const char *arquivo) {
    static char caminho[512];
    const char *raizes[] = {
        "assets/sprites",
        "../assets/sprites",
        "../../assets/sprites"
    };

    for (int i = 0; i < (int)(sizeof(raizes) / sizeof(raizes[0])); i++) {
        snprintf(caminho, sizeof(caminho), "%s/%s", raizes[i], arquivo);

        if (FileExists(caminho)) {
            return caminho;
        }
    }

    snprintf(caminho, sizeof(caminho), "assets/sprites/%s", arquivo);
    return caminho;
}

// Desenha um sprite de 20x20 esticado para uma célula de 40x40.
static void desenharSpriteCelula(Texture2D textura, int quadro,
                                 int coluna, int linha, Color corFallback) {
    int x = coluna * TAMANHO_CELULA;
    int y = linha * TAMANHO_CELULA;

    if (IsTextureValid(textura)) {
        Rectangle origem = {
            (float)(quadro * TAMANHO_SPRITE), 0.0f,
            (float)TAMANHO_SPRITE, (float)TAMANHO_SPRITE
        };
        Rectangle destino = {
            (float)x, (float)y,
            (float)TAMANHO_CELULA, (float)TAMANHO_CELULA
        };

        DrawTexturePro(textura, origem, destino,
                       (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
    } else if (corFallback.a > 0) {
        DrawRectangle(x, y, TAMANHO_CELULA - 1, TAMANHO_CELULA - 1, corFallback);
    }
}

// Escolhe o sprite do pinguim pela última direção e anima quando ele anda.
static void desenharPinguim(void) {
    int x = jogador.coluna * TAMANHO_CELULA;
    int y = jogador.linha * TAMANHO_CELULA;

    // sombra no chão (dá profundidade ao cartoon)
    DrawEllipse(x + TAMANHO_CELULA / 2, y + 36, 11.0f, 4.0f, Fade(BLACK, 0.18f));

    if (!IsTextureValid(spritePinguim)) {
        DrawRectangle(x, y, TAMANHO_CELULA - 1, TAMANHO_CELULA - 1, YELLOW);
        return;
    }

    int direcao = 0; // 0 frente, 1 costas, 2 esquerda, 3 direita

    if (jogador.direcaoLinha < 0) {
        direcao = 1;
    } else if (jogador.direcaoLinha > 0) {
        direcao = 0;
    } else if (jogador.direcaoColuna < 0) {
        direcao = 2;
    } else if (jogador.direcaoColuna > 0) {
        direcao = 3;
    }

    int andando = (GetTime() - tempoUltimoMovimento) < 0.15;
    int quadro = andando ? ((int)(GetTime() * 8.0) % 2) : 0;
    int indice = direcao * 2 + quadro;

    Rectangle origem = {
        (float)(indice * TAMANHO_SPRITE), 0.0f,
        (float)TAMANHO_SPRITE, (float)TAMANHO_SPRITE
    };
    Rectangle destino = {
        (float)x, (float)y,
        (float)TAMANHO_CELULA, (float)TAMANHO_CELULA
    };

    DrawTexturePro(spritePinguim, origem, destino,
                   (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
}

// Desenha os inimigos: foca (patrulhadora) e urso-polar (perseguidor).
// A direcao do olhar e o ritmo do passo sao deduzidos da propria posicao,
// entao nenhuma regra do jogo muda por causa do desenho.
static void desenharInimigos(void) {
    static Posicao posicaoVista[MAX_INIMIGOS];
    static double instantePasso[MAX_INIMIGOS];
    static int direcaoVista[MAX_INIMIGOS];
    static int pronto = 0;

    if (!pronto) {
        for (int i = 0; i < MAX_INIMIGOS; i++) {
            posicaoVista[i] = inimigos[i].posicao;
            direcaoVista[i] = 0;
            instantePasso[i] = -10.0;
        }
        pronto = 1;
    }

    for (int i = 0; i < MAX_INIMIGOS; i++) {
        if (!inimigos[i].ativo) {
            continue;
        }

        int dl = inimigos[i].posicao.linha - posicaoVista[i].linha;
        int dc = inimigos[i].posicao.coluna - posicaoVista[i].coluna;

        if (dl != 0 || dc != 0) {
            // Passos de 1 celula viram direcao + animacao. Saltos grandes
            // (troca de fase) so reposicionam, sem trocar o olhar.
            if (abs(dl) <= 1 && abs(dc) <= 1) {
                if (dc != 0) {
                    direcaoVista[i] = (dc > 0) ? 3 : 2;
                } else {
                    direcaoVista[i] = (dl > 0) ? 0 : 1;
                }

                instantePasso[i] = GetTime();
            }

            posicaoVista[i] = inimigos[i].posicao;
        }

        int x = inimigos[i].posicao.coluna * TAMANHO_CELULA;
        int y = inimigos[i].posicao.linha * TAMANHO_CELULA;

        // sombra no chão (dá profundidade ao cartoon)
        DrawEllipse(x + TAMANHO_CELULA / 2, y + 36, 11.0f, 4.0f,
                    Fade(BLACK, 0.18f));

        Texture2D textura = inimigos[i].perseguidor ? spriteUrso : spriteFoca;

        if (!IsTextureValid(textura)) {
            DrawCircle(x + TAMANHO_CELULA / 2, y + TAMANHO_CELULA / 2,
                       inimigos[i].perseguidor ? 13 : 9,
                       inimigos[i].perseguidor ? RED : ORANGE);
            continue;
        }

        int andando = (GetTime() - instantePasso[i]) < 0.25;
        int quadro = andando ? ((int)(GetTime() * 8.0) % 2) : 0;
        int indice = direcaoVista[i] * 2 + quadro;

        Rectangle origem = {
            (float)(indice * TAMANHO_SPRITE), 0.0f,
            (float)TAMANHO_SPRITE, (float)TAMANHO_SPRITE
        };
        Rectangle destino = {
            (float)x, (float)y,
            (float)TAMANHO_CELULA, (float)TAMANHO_CELULA
        };

        DrawTexturePro(textura, origem, destino,
                       (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
    }
}

// Fundo das telas (menu, recordes, ajustes, fase concluída, game over).
// O "tinta" multiplica as cores: WHITE mantém o original.
static void desenharFundo(Color tinta) {
    ClearBackground(BLACK);

    if (IsTextureValid(spriteFundo)) {
        DrawTexture(spriteFundo, 0, 0, tinta);

        // Escurece de leve pra garantir leitura dos textos.
        DrawRectangle(0, 0, LARGURA_JOGO, ALTURA_JOGO, Fade(BLACK, 0.18f));
    } else {
        ClearBackground(DARKBLUE);
    }
}

// ---- HUD (barra fixa de informações no rodapé) ----

static const Color HUD_TOPO = { 18, 38, 58, 255 };
static const Color HUD_BASE = { 7, 18, 32, 255 };
static const Color HUD_GELO = { 124, 180, 214, 255 };
static const Color HUD_TEXTO = { 236, 247, 255, 255 };
static const Color HUD_TEXTO_FRACO = { 138, 174, 202, 255 };
static const Color HUD_BARRA_FUNDO = { 11, 26, 42, 255 };
static const Color HUD_BARRA_CHEIA = { 96, 214, 246, 255 };
static const Color HUD_BARRA_BRILHO = { 196, 242, 255, 255 };

// Texto alinhado à esquerda, com a mesma sombra usada nas outras telas.
static void desenharTextoSombra(const char *texto, int x, int y,
                                int tamanho, Color cor) {
    DrawText(texto, x + 2, y + 2, tamanho, Fade(BLACK, 0.55f));
    DrawText(texto, x, y, tamanho, cor);
}

// Progresso em blocos: um bloco por peixe da fase.
static void desenharBarraPeixes(int x, int y, int largura, int altura,
                                int coletados, int total) {
    if (total <= 0) {
        return;
    }

    int vao = 2;
    int bloco = (largura - (total - 1) * vao) / total;

    if (bloco < 2) {
        bloco = 2;
    }
    if (total > 20) {
        // Fases com muitos peixes: blocos finos demais viram ruído.
        vao = 1;
        bloco = (largura - (total - 1) * vao) / total;

        if (bloco < 2) {
            bloco = 2;
        }
    }

    for (int i = 0; i < total; i++) {
        int bx = x + i * (bloco + vao);
        Rectangle r = { (float)bx, (float)y, (float)bloco, (float)altura };

        if (i < coletados) {
            DrawRectangleRec(r, HUD_BARRA_CHEIA);
            DrawRectangle(bx, y, bloco, 3, HUD_BARRA_BRILHO);
        } else {
            DrawRectangleRec(r, HUD_BARRA_FUNDO);
            DrawRectangleLinesEx(r, 1.0f, Fade(HUD_GELO, 0.22f));
        }
    }
}

// Painel de gelo com peixes coletados, pontos, progresso, fase e controles.
static void desenharHUD(void) {
    int y0 = LINHAS * TAMANHO_CELULA;
    int coletados = contarFrutasColetadas();

    // Painel com gradiente, aro de gelo no topo e sombra embaixo.
    DrawRectangleGradientV(0, y0, LARGURA_JOGO, 60, HUD_TOPO, HUD_BASE);
    DrawRectangle(0, y0, LARGURA_JOGO, 2, Fade(HUD_GELO, 0.85f));
    DrawRectangle(0, y0 + 2, LARGURA_JOGO, 1, Fade(HUD_GELO, 0.25f));
    DrawRectangle(0, y0 + 57, LARGURA_JOGO, 3, Fade(BLACK, 0.55f));

    // Poeira de gelo no painel (posições fixas, sem piscar).
    for (int i = 0; i < 46; i++) {
        int px = (i * 197 + 13) % (LARGURA_JOGO - 16) + 8;
        int py = y0 + 6 + (i * 53) % 46;

        DrawPixel(px, py, Fade(WHITE, 0.06f + (float)(i % 4) * 0.03f));
    }

    // --- esquerda: peixe animado + coletados/total ---
    if (IsTextureValid(spritePeixe)) {
        int quadro = ((int)(GetTime() * 5.0)) % QUADROS_PEIXE;
        Rectangle origem = {
            (float)(quadro * TAMANHO_SPRITE), 0.0f,
            (float)TAMANHO_SPRITE, (float)TAMANHO_SPRITE
        };
        Rectangle destino = { 20.0f, (float)(y0 + 10), 40.0f, 40.0f };

        DrawCircle(40, y0 + 30, 21, Fade(BLACK, 0.20f));
        DrawTexturePro(spritePeixe, origem, destino,
                       (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
    }

    const char *textoColetados = TextFormat("%d", coletados);

    desenharTextoSombra(textoColetados, 72, y0 + 12, 28, HUD_TEXTO);
    desenharTextoSombra(TextFormat("/%d", quantidadeFrutasAtivas),
                        72 + MeasureText(textoColetados, 28) + 4, y0 + 20,
                        18, HUD_TEXTO_FRACO);

    DrawRectangle(200, y0 + 12, 1, 36, Fade(HUD_GELO, 0.22f));

    // --- meio: pontuação ---
    desenharTextoSombra("PONTOS", 224, y0 + 6, 10, HUD_TEXTO_FRACO);
    desenharTextoSombra(TextFormat("%06d", jogo.pontos), 224, y0 + 18, 28,
                        HUD_TEXTO);

    DrawRectangle(400, y0 + 12, 1, 36, Fade(HUD_GELO, 0.22f));

    // --- progresso em blocos ---
    desenharTextoSombra("PEIXES", 424, y0 + 6, 10, HUD_TEXTO_FRACO);
    desenharBarraPeixes(424, y0 + 25, 280, 12, coletados,
                        quantidadeFrutasAtivas);

    // --- direita: fase e controles ---
    const char *textoFase = TextFormat("FASE %d", jogo.fase);
    int badgeLargura = MeasureText(textoFase, 16) + 36;
    Rectangle badge = {
        (float)(LARGURA_JOGO - 16 - badgeLargura), (float)(y0 + 10),
        (float)badgeLargura, 26.0f
    };

    DrawRectangleRounded(badge, 0.45f, 4, Fade(HUD_GELO, 0.14f));
    DrawRectangleRoundedLinesEx(badge, 0.45f, 4, 1.0f,
                                Fade(HUD_GELO, 0.45f));
    desenharTextoSombra(textoFase,
                        (int)(badge.x +
                              (badge.width - MeasureText(textoFase, 16)) / 2),
                        y0 + 15, 16, HUD_TEXTO);

    const char *controles = "WASD mover  |  K gelo  |  P reconstruir";

    desenharTextoSombra(controles,
                        LARGURA_JOGO - 16 - MeasureText(controles, 13),
                        y0 + 41, 13, HUD_TEXTO_FRACO);
}

void desenharJogo(void) {
    BeginTextureMode(alvo);
    ClearBackground(DARKBLUE);

    // Chão de neve em todas as células.
    for (int l = 0; l < LINHAS; l++) {
        for (int c = 0; c < COLUNAS; c++) {
            desenharSpriteCelula(spriteChao, 0, c, l, BLANK);
        }
    }

    // Paredes de pedra e blocos de gelo.
    for (int l = 0; l < LINHAS; l++) {
        for (int c = 0; c < COLUNAS; c++) {
            if (mapa[l][c] == PAREDE_NORMAL) {
                desenharSpriteCelula(spriteParede, 0, c, l, GRAY);
            } else if (mapa[l][c] == PAREDE_GELO) {
                int variante = (l * 7 + c * 13) % QUADROS_GELO;
                desenharSpriteCelula(spriteGelo, variante, c, l, SKYBLUE);
            }
        }
    }

    // Peixes (itens coletáveis) ainda não coletados.
    for (int i = indiceInicialFrutas;
     i < indiceInicialFrutas + quantidadeFrutasAtivas;
     i++) {

        if (!frutaColetada[i]) {
            int x = frutas[i].coluna * TAMANHO_CELULA;
            int y = frutas[i].linha * TAMANHO_CELULA;

            if (IsTextureValid(spritePeixe)) {
                // Cada peixe nada com um pequeno atraso em relação ao vizinho.
                int quadro = ((int)(GetTime() * 6.0) +
                              frutas[i].coluna + frutas[i].linha) % QUADROS_PEIXE;

                DrawEllipse(x + TAMANHO_CELULA / 2, y + 35, 9.0f, 3.5f,
                            Fade(BLACK, 0.16f));

                desenharSpriteCelula(spritePeixe, quadro,
                                     frutas[i].coluna, frutas[i].linha, BLANK);
            } else {
                DrawCircle(
                    x + TAMANHO_CELULA / 2,
                    y + TAMANHO_CELULA / 2,
                    8,
                    RED
                );

                DrawCircle(x + 18, y + 9, 3, GREEN);
            }
        }
    }

    // Inimigos: foca (patrulha) e urso-polar (perseguidor).
    desenharInimigos();

    // Jogador (pinguim).
    desenharPinguim();

    // Barra fixa de informações no rodapé.
    desenharHUD();

    EndTextureMode();
}

// Desenha um texto centralizado horizontalmente na janela.
void desenharTextoCentro(const char *texto, int y, int tamanho, Color cor) {
    int x = (GetRenderWidth() - MeasureText(texto, tamanho)) / 2;

    // Sombra curta: garante leitura sobre o fundo nevado.
    DrawText(texto, x + 2, y + 2, tamanho, Fade(BLACK, 0.55f));
    DrawText(texto, x, y, tamanho, cor);
}

// Título com sombra escura atrás: fica legível sobre o fundo nevado.
void desenharTitulo(const char *texto, int y, int tamanho, Color cor) {
    desenharTextoCentro(texto, y + 3, tamanho, Fade(BLACK, 0.55f));
    desenharTextoCentro(texto, y, tamanho, cor);
}

// Coloca o jogo no começo de uma fase (serve pra começar,
// reiniciar e passar de fase).
void iniciarFase(void) {
    jogador = (Jogador){1, 1, 0, 1};

    const Fase *config = obterFase(jogo.fase);

    if (config == NULL) {
        return;
    }

    indiceInicialFrutas = 0;
    segundaLevaFase2 = 0;
    terceiraLevaFase3 = 0;

    quantidadeFrutasAtivas = config->frutasPrimeiraLeva;

    for (int i = 0; i < MAX_FRUTAS; i++) {
        frutaColetada[i] = 0;
    }

    if (config->mapa == 1) {
        inicializarMapaFase1();
    }
    else if (config->mapa == 2) {
        inicializarMapaFase2();
    }
    else if (config->mapa == 3) {
        inicializarMapaFase3();
    }

    inicializarInimigos();
    prepararRotasInimigos();

    // Barreira inicial de gelo
    for (int c = 1; c < COLUNAS - 1; c++) {
        if (mapa[5][c] != PAREDE_NORMAL) {
            mapa[5][c] = PAREDE_GELO;
        }
    }
}

// Insere um recorde mantendo a lista ordenada (maior pontuação primeiro).
void inserirRecorde(int pontos, int fase) {
    Recorde *novo = malloc(sizeof(Recorde));
    if (novo == NULL) return;   // malloc falhou

    novo->pontos = pontos;
    novo->fase = fase;
    novo->prox = NULL;

    // Lista vazia ou novo é o maior: entra no início.
    if (recordes == NULL || pontos > recordes->pontos) {
        novo->prox = recordes;
        recordes = novo;
        return;
    }

    // Senão, anda até achar o lugar certo.
    Recorde *atual = recordes;
    while (atual->prox != NULL && atual->prox->pontos >= pontos) {
        atual = atual->prox;
    }
    novo->prox = atual->prox;
    atual->prox = novo;
}

void liberarRecordes(void) {
    while (recordes != NULL) {
        Recorde *prox = recordes->prox;
        free(recordes);
        recordes = prox;
    }
}

// Carrega os recordes salvos e mantém a lista ordenada.
void carregarRecordes(void) {
    FILE *arquivo = fopen(ARQUIVO_RECORDE, "r");
    if (arquivo == NULL) {
        return; // Ainda não existe arquivo de recordes.
    }

    int pontos;
    int fase;
    while (fscanf(arquivo, "%d %d", &pontos, &fase) == 2) {
        if (pontos >= 0 && fase > 0) {
            inserirRecorde(pontos, fase);
        }
    }

    fclose(arquivo);
}

// Grava a lista completa em texto, um recorde por linha: pontos fase.
void salvarRecordes(void) {
    FILE *arquivo = fopen(ARQUIVO_RECORDE, "w");
    if (arquivo == NULL) {
        return;
    }

    for (Recorde *r = recordes; r != NULL; r = r->prox) {
        fprintf(arquivo, "%d %d\n", r->pontos, r->fase);
    }

    fclose(arquivo);
}

// Lógica de um frame do jogo (sem desenhar).
void atualizarJogo(void) {
    double intervaloMovimento = 0.13;

    if (GetTime() - tempoUltimoMovimento >= intervaloMovimento) {

        if (IsKeyDown(KEY_W)) {
            moverJogador(-1, 0);
            tempoUltimoMovimento = GetTime();
        }
        else if (IsKeyDown(KEY_S)) {
            moverJogador(1, 0);
            tempoUltimoMovimento = GetTime();
        }
        else if (IsKeyDown(KEY_A)) {
            moverJogador(0, -1);
            tempoUltimoMovimento = GetTime();
        }
        else if (IsKeyDown(KEY_D)) {
            moverJogador(0, 1);
            tempoUltimoMovimento = GetTime();
        }
    }

    if (IsKeyPressed(KEY_K)) quebrarGelo();
    if (IsKeyPressed(KEY_P)) reconstruirGelo();

    moverInimigos();
    moverPerseguidor();

    if (jogadorAtingido()) {
        inserirRecorde(jogo.pontos, jogo.fase);
        salvarRecordes();
        jogo.estado = GAME_OVER;
        return;
    }

    const Fase *config = obterFase(jogo.fase);

    if (config != NULL && contarFrutasColetadas() == quantidadeFrutasAtivas) {

        // Inicia a segunda leva.
        if (segundaLevaFase2 == 0 &&
            config->frutasSegundaLeva > 0) {

            segundaLevaFase2 = 1;

            indiceInicialFrutas = config->frutasPrimeiraLeva;
            quantidadeFrutasAtivas = config->frutasSegundaLeva;

        }
        // Inicia a terceira leva.
        else if (terceiraLevaFase3 == 0 &&
                config->frutasTerceiraLeva > 0) {

            terceiraLevaFase3 = 1;

            indiceInicialFrutas =
                config->frutasPrimeiraLeva +
                config->frutasSegundaLeva;

            quantidadeFrutasAtivas = config->frutasTerceiraLeva;
        }
        // Todas as levas foram concluídas.
        else {
            jogo.estado = FASE_CONCLUIDA;
        }

        // Reinicia o estado das frutas da nova leva.
        if (jogo.estado != FASE_CONCLUIDA) {
            for (int i = indiceInicialFrutas;
                i < indiceInicialFrutas + quantidadeFrutasAtivas;
                i++) {

                frutaColetada[i] = 0;
            }
        }
    }
}

// Tela de fase concluída: ENTER vai pra próxima fase.
void telaFaseConcluida(void) {
    if (IsKeyPressed(KEY_ENTER)) {
        jogo.fase++;
        iniciarFase();          // por enquanto repete o mesmo mapa
        jogo.estado = JOGANDO;
    }

    BeginTextureMode(alvo);
    desenharFundo(WHITE);
    desenharTitulo("FASE CONCLUIDA!", 90, 35, GREEN);
    desenharTextoCentro(TextFormat("Pontos: %d", jogo.pontos), 160, 22, RAYWHITE);
    desenharTextoCentro(TextFormat("Peixes: %d/%d", contarFrutasColetadas(), quantidadeFrutasAtivas),
                    195, 22, RAYWHITE);
    desenharTextoCentro("ENTER: proxima fase", 270, 18, RAYWHITE);
    EndTextureMode();
}

// Tela de game over: ENTER recomeça do zero.
void telaGameOver(void) {
    if (IsKeyPressed(KEY_ENTER)) {
        jogo.pontos = 0;
        jogo.fase = 1;
        iniciarFase();
        jogo.estado = MENU;  
    }

    BeginTextureMode(alvo);
    desenharFundo((Color){ 255, 176, 176, 255 });
    desenharTitulo("FIM DE JOGO!", 90, 35, RED);
    desenharTextoCentro("Voce foi alcancado por um inimigo.", 140, 20, RAYWHITE);
    desenharTextoCentro(TextFormat("Pontos: %d", jogo.pontos), 190, 22, RAYWHITE);
    desenharTextoCentro(TextFormat("Peixes: %d/%d", contarFrutasColetadas(), MAX_FRUTAS),
                        225, 22, RAYWHITE);
    desenharTextoCentro("ENTER: voltar ao menu", 290, 18, RAYWHITE);
    
    EndTextureMode();
}

void telaMenu(void) {
     const char *opcoes[NUM_OPCOES_MENU] = {"Jogar", "Recordes", "Ajustes", "Sair"};

    // Navegação com setas ou W/S
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        opcaoMenu = (opcaoMenu + 1) % NUM_OPCOES_MENU;
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        opcaoMenu = (opcaoMenu + NUM_OPCOES_MENU - 1) % NUM_OPCOES_MENU;
    }

    // Confirmação
    if (IsKeyPressed(KEY_ENTER)) {
        if (opcaoMenu == 0) {
            jogo.pontos = 0;
            jogo.fase = 1;
            iniciarFase();
            jogo.estado = JOGANDO;
        } else if (opcaoMenu == 1) {
            jogo.estado = RECORDES;
        } else if (opcaoMenu == 2) {
            jogo.estado = AJUSTES;
        } else {
            jogo.estado = SAIR;
        }
    }

    BeginTextureMode(alvo);
    desenharFundo(WHITE);

    // Painel escuro atrás das opções, pra destacar do fundo nevado.
    DrawRectangleRounded((Rectangle){ 300, 138, 600, 205 }, 0.08f, 6,
                         Fade(BLACK, 0.34f));

    if (IsTextureValid(logoMenu)) {
        float larguraLogo = 650.0f;
        float alturaLogo = larguraLogo * (float)logoMenu.height / logoMenu.width;
        DrawTexturePro(logoMenu,
                       (Rectangle){ 0, 0, (float)logoMenu.width, (float)logoMenu.height },
                       (Rectangle){ (LARGURA_JOGO - larguraLogo) / 2.0f, 42,
                                    larguraLogo, alturaLogo },
                       (Vector2){ 0, 0 }, 0, WHITE);
    } else {
        Image imagem = LoadImage(caminhoSprite("logo_frostbite.jpg"));
        if (imagem.data != NULL) {
            ImageCrop(&imagem, (Rectangle){
                imagem.width * 0.03f, imagem.height * 0.27f,
                imagem.width * 0.94f, imagem.height * 0.46f
            });
            ImageFormat(&imagem, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
            ImageColorReplace(&imagem, BLACK, BLANK);
            logoMenu = LoadTextureFromImage(imagem);
            UnloadImage(imagem);

            if (IsTextureValid(logoMenu)) {
                float larguraLogo = 650.0f;
                float alturaLogo = larguraLogo * (float)logoMenu.height / logoMenu.width;
                DrawTexturePro(logoMenu,
                               (Rectangle){ 0, 0, (float)logoMenu.width, (float)logoMenu.height },
                               (Rectangle){ (LARGURA_JOGO - larguraLogo) / 2.0f, 42,
                                            larguraLogo, alturaLogo },
                               (Vector2){ 0, 0 }, 0, WHITE);
            } else {
                desenharTitulo("FROSTBITE", 50, 50, SKYBLUE);
            }
        } else {
            desenharTitulo("FROSTBITE", 50, 50, SKYBLUE);
        }
    }

    for (int i = 0; i < NUM_OPCOES_MENU; i++) {
        if (i == opcaoMenu) {
            desenharTextoCentro(TextFormat("> %s <", opcoes[i]),
                                160 + i * 45, 28, YELLOW);
        } else {
            desenharTextoCentro(opcoes[i], 160 + i * 45, 28, RAYWHITE);
        }
    }

    desenharTextoCentro("Setas ou W/S: escolher | ENTER: confirmar",
                        GetRenderHeight() - 40, 16, RAYWHITE);
    EndTextureMode();
}

// Ajusta o volume da música pelas setas esquerda e direita.
void telaAjustes(Music musica) {
    if (IsKeyPressed(KEY_LEFT) && volumeMusica > 0.0f) {
        volumeMusica -= 0.1f;
        if (volumeMusica < 0.05f) volumeMusica = 0.0f;
    }
    if (IsKeyPressed(KEY_RIGHT) && volumeMusica < 1.0f) {
        volumeMusica += 0.1f;
        if (volumeMusica > 0.95f) volumeMusica = 1.0f;
    }

    if (IsMusicValid(musica)) {
        SetMusicVolume(musica, volumeMusica);
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_BACKSPACE)) {
        jogo.estado = MENU;
    }

    BeginTextureMode(alvo);
    desenharFundo(WHITE);
    desenharTitulo("AJUSTES", 50, 40, GOLD);
    desenharTextoCentro("Volume da musica", 140, 26, RAYWHITE);
    desenharTextoCentro(TextFormat("%d%%", (int)(volumeMusica * 100.0f + 0.5f)),
                        180, 24, YELLOW);

    Rectangle barra = { 390.0f, 225.0f, 420.0f, 24.0f };
    DrawRectangleRounded(barra, 0.4f, 8, Fade(BLACK, 0.45f));
    if (volumeMusica > 0.0f) {
        Rectangle preenchimento = {
            barra.x, barra.y, barra.width * volumeMusica, barra.height
        };
        DrawRectangleRounded(preenchimento, 0.4f, 8, SKYBLUE);
    }

    desenharTextoCentro("SETA ESQUERDA: diminuir | SETA DIREITA: aumentar",
                        285, 18, RAYWHITE);
    desenharTextoCentro("0% desliga a musica", 315, 16, LIGHTGRAY);
    desenharTextoCentro("ENTER ou BACKSPACE: voltar ao menu",
                        GetRenderHeight() - 40, 16, RAYWHITE);
    EndTextureMode();
}

// Mostra o top 5 lido da lista de recordes.
void telaRecordes(void) {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_BACKSPACE)) {
        jogo.estado = MENU;
    }

    BeginTextureMode(alvo);
    desenharFundo(WHITE);

    // Painel escuro atrás da lista, pra destacar do fundo nevado.
    DrawRectangleRounded((Rectangle){ 250, 120, 700, 215 }, 0.08f, 6,
                         Fade(BLACK, 0.34f));

    desenharTitulo("RECORDES", 50, 40, GOLD);

    if (recordes == NULL) {
        desenharTextoCentro("Nenhum recorde ainda.", 160, 22, RAYWHITE);
    } else {
        int pos = 0;
        for (Recorde *r = recordes; r != NULL && pos < 5; r = r->prox, pos++) {
            desenharTextoCentro(
                TextFormat("%d. %d pontos (fase %d)", pos + 1, r->pontos, r->fase),
                140 + pos * 40, 24, RAYWHITE
            );
        }
    }

    desenharTextoCentro("ENTER: voltar ao menu", GetRenderHeight() - 40, 16, RAYWHITE);
    EndTextureMode();
}

// Procura a música em caminhos comuns, para funcionar tanto rodando
// da raiz do projeto quanto de dentro de src/.
static const char *caminhoDaMusica(void) {
    const char *opcoes[] = {
        "assets/musica.mp3",
        "../assets/musica.mp3",
        "src/musica.mp3"
    };

    for (int i = 0; i < (int)(sizeof(opcoes) / sizeof(opcoes[0])); i++) {
        if (FileExists(opcoes[i])) {
            return opcoes[i];
        }
    }

    return opcoes[0]; // não achou: o jogo segue sem som
}

// Desenha a textura do jogo centralizada na janela, mantendo a proporção.
void desenharTelaEscalada(void) {
    float escalaX = (float)GetScreenWidth() / LARGURA_JOGO;
    float escalaY = (float)GetScreenHeight() / ALTURA_JOGO;
    float escala = (escalaX < escalaY) ? escalaX : escalaY;

    float largura = LARGURA_JOGO * escala;
    float altura = ALTURA_JOGO * escala;
    float x = (GetScreenWidth() - largura) / 2.0f;
    float y = (GetScreenHeight() - altura) / 2.0f;

    DrawTexturePro(
        alvo.texture,
        (Rectangle){ 0.0f, 0.0f, (float)alvo.texture.width, -(float)alvo.texture.height },
        (Rectangle){ x, y, largura, altura },
        (Vector2){ 0.0f, 0.0f },
        0.0f,
        WHITE
    );
}

int main(void) {
    InitWindow(LARGURA_JOGO, ALTURA_JOGO, "FROSTBITE");
    ToggleFullscreen();

    // O jogo é desenhado nessa textura (resolução fixa) e escalado para a tela.
    // Filtro POINT mantém os pixels nítidos (sem borrar o pixel art).
    alvo = LoadRenderTexture(LARGURA_JOGO, ALTURA_JOGO);
    SetTextureFilter(alvo.texture, TEXTURE_FILTER_POINT);

    // Sprites do design (pixel art). Se faltar algum, o desenho cai no
    // retângulo colorido antigo (não quebra o jogo).
    spriteChao = LoadTexture(caminhoSprite("chao.png"));
    spriteParede = LoadTexture(caminhoSprite("parede.png"));
    spriteGelo = LoadTexture(caminhoSprite("gelo.png"));
    spritePinguim = LoadTexture(caminhoSprite("pinguim.png"));
    spriteFoca = LoadTexture(caminhoSprite("foca.png"));
    spriteUrso = LoadTexture(caminhoSprite("urso.png"));
    spritePeixe = LoadTexture(caminhoSprite("peixe.png"));
    spriteFundo = LoadTexture(caminhoSprite("fundo.png"));

    Texture2D sprites[] = {
        spriteChao, spriteParede, spriteGelo, spritePinguim,
        spriteFoca, spriteUrso, spritePeixe, spriteFundo
    };

    for (int i = 0; i < (int)(sizeof(sprites) / sizeof(sprites[0])); i++) {
        if (IsTextureValid(sprites[i])) {
            SetTextureFilter(sprites[i], TEXTURE_FILTER_POINT);
        }
    }

    jogo.estado = MENU;
    jogo.pontos = 0;
    jogo.fase = 1;
    carregarRecordes();

    SetTargetFPS(60);

    InitAudioDevice(); // Liga o sistema de som
    Music musica = LoadMusicStream(caminhoDaMusica());
    if (IsMusicValid(musica)) {
        SetMusicVolume(musica, volumeMusica);
        PlayMusicStream(musica);
    } else {
        TraceLog(LOG_WARNING, "MUSICA: nao foi possivel carregar assets/musica.mp3");
    }

    while (!WindowShouldClose() && jogo.estado != SAIR) {
        if (IsKeyPressed(KEY_F11) || (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_ENTER))) {
            ToggleFullscreen();
        }

        if (IsMusicValid(musica)) {
            UpdateMusicStream(musica); // sem isso a música nunca avança

            if (!IsMusicStreamPlaying(musica)) { // terminou: começa de novo
                SeekMusicStream(musica, 0.0f);
                PlayMusicStream(musica);
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        switch (jogo.estado) {
            case MENU:
                telaMenu();
                break;
            case JOGANDO:
                atualizarJogo();
                desenharJogo();
                break;
            case FASE_CONCLUIDA:
                telaFaseConcluida();
                break;
            case GAME_OVER:
                telaGameOver();
                break;
            case AJUSTES:
                telaAjustes(musica);
                break;
            case RECORDES:
                telaRecordes();
                break;
            default:
                jogo.estado = SAIR;
                break;
        }

        desenharTelaEscalada();
        EndDrawing();
    }

    if (IsMusicValid(musica)) {
        UnloadMusicStream(musica);
    }
    CloseAudioDevice();
    UnloadTexture(spriteChao);
    UnloadTexture(spriteParede);
    UnloadTexture(spriteGelo);
    UnloadTexture(spritePinguim);
    UnloadTexture(spriteFoca);
    UnloadTexture(spriteUrso);
    UnloadTexture(spritePeixe);
    UnloadTexture(spriteFundo);
    if (IsTextureValid(logoMenu)) UnloadTexture(logoMenu);
    UnloadRenderTexture(alvo);
    liberarRecordes();
    CloseWindow();
    return 0;
}
