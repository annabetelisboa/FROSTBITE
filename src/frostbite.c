#include <stdlib.h>
#include "raylib.h"

void preencherMapaBase(void);
void abrirCorredor(int linha, int inicio, int fim);

#define LINHAS 15
#define COLUNAS 30
#define TAMANHO_CELULA 40

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
} Recorde;

Recorde *recordes = NULL;

char mapa[LINHAS][COLUNAS];
Jogador jogador = {1, 1, 0, 1};

Posicao frutas[MAX_FRUTAS] = {
    // Primeira leva: 13 frutas
    {1, 5}, {2, 12}, {4, 3},
    {4, 17}, {6, 24}, {8, 10},
    {10, 5}, {12, 19}, {13, 26},
    {2, 20}, {6, 14}, {9, 25},
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

void desenharJogo(void) {
    BeginDrawing();
    ClearBackground(DARKBLUE);

    // Paredes.
    for (int l = 0; l < LINHAS; l++) {
        for (int c = 0; c < COLUNAS; c++) {
            int x = c * TAMANHO_CELULA;
            int y = l * TAMANHO_CELULA;

            if (mapa[l][c] == PAREDE_NORMAL) {
                DrawRectangle(
                    x, y,
                    TAMANHO_CELULA - 1,
                    TAMANHO_CELULA - 1,
                    GRAY
                );
            } else if (mapa[l][c] == PAREDE_GELO) {
                DrawRectangle(
                    x, y,
                    TAMANHO_CELULA - 1,
                    TAMANHO_CELULA - 1,
                    SKYBLUE
                );
            }
        }
    }

    // Frutas ainda não coletadas.
    for (int i = indiceInicialFrutas;
     i < indiceInicialFrutas + quantidadeFrutasAtivas;
     i++) {

        if (!frutaColetada[i]) {
            int x = frutas[i].coluna * TAMANHO_CELULA;
            int y = frutas[i].linha * TAMANHO_CELULA;

            DrawCircle(
                x + TAMANHO_CELULA / 2,
                y + TAMANHO_CELULA / 2,
                8,
                RED
            );

            DrawCircle(x + 18, y + 9, 3, GREEN);
        }
    }

    // Inimigos.
    for (int i = 0; i < MAX_INIMIGOS; i++) {
        if (!inimigos[i].ativo) {
            continue;
        }
        int x = inimigos[i].posicao.coluna * TAMANHO_CELULA;
        int y = inimigos[i].posicao.linha * TAMANHO_CELULA;

        if (inimigos[i].perseguidor) {
            DrawCircle(
                x + TAMANHO_CELULA / 2,
                y + TAMANHO_CELULA / 2,
                13,
                RED
            );
        } 
        else {
            DrawCircle(
                x + TAMANHO_CELULA / 2,
                y + TAMANHO_CELULA / 2,
                9,
                ORANGE
            );
        }
    }

    // Jogador.
    DrawRectangle(
        jogador.coluna * TAMANHO_CELULA,
        jogador.linha * TAMANHO_CELULA,
        TAMANHO_CELULA - 1,
        TAMANHO_CELULA - 1,
        YELLOW
    );

    DrawRectangle(
        0, LINHAS * TAMANHO_CELULA,
        COLUNAS * TAMANHO_CELULA, 60,
        BLACK
    );

    DrawText(
        TextFormat("Frutas: %d/%d",
                   contarFrutasColetadas(), quantidadeFrutasAtivas),
        15, LINHAS * TAMANHO_CELULA + 8,
        18, RAYWHITE
    );

    DrawText(
    TextFormat("Pontos: %d", jogo.pontos),
        200, LINHAS * TAMANHO_CELULA + 8,
        18, RAYWHITE
    );

    DrawText(
        "WASD: mover | K: quebrar gelo | P: reconstruir",
        15, LINHAS * TAMANHO_CELULA + 32,
        15, RAYWHITE
    );

    EndDrawing();
}

// Desenha um texto centralizado horizontalmente na janela.
void desenharTextoCentro(const char *texto, int y, int tamanho, Color cor) {
    int x = (GetScreenWidth() - MeasureText(texto, tamanho)) / 2;
    DrawText(texto, x, y, tamanho, cor);
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

    BeginDrawing();
    ClearBackground(DARKBLUE);
    desenharTextoCentro("FASE CONCLUIDA!", 90, 35, GREEN);
    desenharTextoCentro(TextFormat("Pontos: %d", jogo.pontos), 160, 22, RAYWHITE);
    desenharTextoCentro(TextFormat("Frutas: %d/%d", contarFrutasColetadas(), quantidadeFrutasAtivas),
                    195, 22, RAYWHITE);
    desenharTextoCentro("ENTER: proxima fase", 270, 18, RAYWHITE);
    EndDrawing();
}

// Tela de game over: ENTER recomeça do zero.
void telaGameOver(void) {
    if (IsKeyPressed(KEY_ENTER)) {
        jogo.pontos = 0;
        jogo.fase = 1;
        iniciarFase();
        jogo.estado = MENU;  
    }

    BeginDrawing();
    ClearBackground(DARKBLUE);
    desenharTextoCentro("FIM DE JOGO!", 90, 35, RED);
    desenharTextoCentro("Voce foi alcancado por um inimigo.", 140, 20, RAYWHITE);
    desenharTextoCentro(TextFormat("Pontos: %d", jogo.pontos), 190, 22, RAYWHITE);
    desenharTextoCentro(TextFormat("Frutas: %d/%d", contarFrutasColetadas(), MAX_FRUTAS),
                        225, 22, RAYWHITE);
    desenharTextoCentro("ENTER: voltar ao menu", 290, 18, RAYWHITE);
    
    EndDrawing();
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

    BeginDrawing();
    ClearBackground(DARKBLUE);
    desenharTextoCentro("FROSTBITE", 50, 50, SKYBLUE);

    for (int i = 0; i < NUM_OPCOES_MENU; i++) {
        if (i == opcaoMenu) {
            desenharTextoCentro(TextFormat("> %s <", opcoes[i]),
                                160 + i * 45, 28, YELLOW);
        } else {
            desenharTextoCentro(opcoes[i], 160 + i * 45, 28, RAYWHITE);
        }
    }

    desenharTextoCentro("Setas ou W/S: escolher | ENTER: confirmar",
                        GetScreenHeight() - 40, 16, RAYWHITE);
    EndDrawing();
}

// Tela de ajustes (vazia por enquanto; o arquivo vem depois).
void telaAjustes(void) {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_BACKSPACE)) {
        jogo.estado = MENU;
    }

    BeginDrawing();
    ClearBackground(DARKBLUE);
    desenharTextoCentro("AJUSTES", 50, 40, GOLD);
    desenharTextoCentro("Nenhum ainda.", 160, 22, RAYWHITE);
    desenharTextoCentro("ENTER: voltar ao menu", GetScreenHeight() - 40, 16, RAYWHITE);
    EndDrawing();
}

// Mostra o top 5 lido da lista de recordes.
void telaRecordes(void) {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_BACKSPACE)) {
        jogo.estado = MENU;
    }

    BeginDrawing();
    ClearBackground(DARKBLUE);
    desenharTextoCentro("RECORDES", 50, 40, GOLD);

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

    desenharTextoCentro("ENTER: voltar ao menu", GetScreenHeight() - 40, 16, RAYWHITE);
    EndDrawing();
}

int main(void) {
    InitWindow(
        COLUNAS * TAMANHO_CELULA,
        LINHAS * TAMANHO_CELULA + 60,
        "FROSTBITE"
    );

    jogo.estado = MENU;
    jogo.pontos = 0;
    jogo.fase = 1;

    SetTargetFPS(60);

    while (!WindowShouldClose() && jogo.estado != SAIR) {
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
                telaAjustes();
                break;
            case RECORDES:
                telaRecordes();
                break;
            default:
                jogo.estado = SAIR;
                break;
        }
    }
    liberarRecordes();
    CloseWindow();
    return 0;
}