
#include <stdio.h>
#include "screen.h"
#include "keyboard.h"

#define LINHAS 15
#define COLUNAS 30

#define PAREDE_NORMAL '#'
#define PAREDE_GELO 'G'
#define CAMINHO ' '

#define JOGADOR 'P'

typedef struct {
    int linha;
    int coluna;
    int direcaoLinha;
    int direcaoColuna;
} Jogador;

char mapa[LINHAS][COLUNAS];

Jogador jogador = {1, 1, 0, 1};

/* Usa o fundo padrão do terminal quando o fundo pedido é BLACK */
static void cor(int fg, int bg) {
    screenSetColor(fg, bg);
    if (bg == BLACK) {
        printf("\033[49m");
    }
}

/* Posiciona o cursor de forma absoluta (coluna x, linha y, começando em 1) */
static void ir(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

#define MOLD_ESQ   3
#define MOLD_DIR   52
#define MOLD_TOPO  1
#define MOLD_BASE  22

static void desenharMoldura(void) {
    cor(WHITE, BLACK);

    ir(MOLD_ESQ, MOLD_TOPO);
    printf("┌");
    for (int x = MOLD_ESQ + 1; x < MOLD_DIR; x++) printf("─");
    printf("┐");

    for (int y = MOLD_TOPO + 1; y < MOLD_BASE; y++) {
        ir(MOLD_ESQ, y);
        printf("│");
        ir(MOLD_DIR, y);
        printf("│");
    }

    ir(MOLD_ESQ, MOLD_BASE);
    printf("└");
    for (int x = MOLD_ESQ + 1; x < MOLD_DIR; x++) printf("─");
    printf("┘");
}

void inicializarMapa(void) {
    for (int linha = 0; linha < LINHAS; linha++) {
        for (int coluna = 0; coluna < COLUNAS; coluna++) {
            if (linha == 0 || linha == LINHAS - 1 ||
                coluna == 0 || coluna == COLUNAS - 1) {
                mapa[linha][coluna] = PAREDE_NORMAL;
            } else {
                mapa[linha][coluna] = CAMINHO;
            }
        }
    }

    /* Paredes normais internas */
    for (int coluna = 5; coluna <= 10; coluna++) {
        mapa[3][coluna] = PAREDE_NORMAL;
    }

    for (int linha = 5; linha <= 9; linha++) {
        mapa[linha][15] = PAREDE_NORMAL;
    }

    /* Paredes de gelo */
    mapa[2][4] = PAREDE_GELO;
    mapa[2][5] = PAREDE_GELO;
    mapa[4][12] = PAREDE_GELO;
    mapa[4][13] = PAREDE_GELO;
    mapa[10][20] = PAREDE_GELO;
    mapa[10][21] = PAREDE_GELO;
}


void desenharMapa(void) {
    screenSetNormal();
    screenClear();

    desenharMoldura();

    cor(WHITE, BLACK);
    ir(5, 2);
    printf("FROSTBITE");

    for (int linha = 0; linha < LINHAS; linha++) {
        ir(5, linha + 4);
        cor(WHITE, BLACK);

        for (int coluna = 0; coluna < COLUNAS; coluna++) {
            if (linha == jogador.linha && coluna == jogador.coluna) {
                cor(YELLOW, BLACK);
                printf("%c", JOGADOR);
            } else if (mapa[linha][coluna] == PAREDE_NORMAL) {
                cor(WHITE, BLACK);
                printf("#");
            } else if (mapa[linha][coluna] == PAREDE_GELO) {
                cor(CYAN, BLUE);
                printf("G");
            } else {
                cor(WHITE, BLACK);
                printf(" ");
            }
        }
    }

    cor(WHITE, BLACK);
    ir(5, 20);
    printf("WASD: mover | B: quebrar gelo | R: reconstruir");

    ir(5, 21);
    printf("Q: sair                                      ");

    screenUpdate();
}

void atualizarDirecao(int tecla) {
    switch (tecla) {
        case 'w':
        case 'W':
            jogador.direcaoLinha = -1;
            jogador.direcaoColuna = 0;
            break;

        case 's':
        case 'S':
            jogador.direcaoLinha = 1;
            jogador.direcaoColuna = 0;
            break;

        case 'a':
        case 'A':
            jogador.direcaoLinha = 0;
            jogador.direcaoColuna = -1;
            break;

        case 'd':
        case 'D':
            jogador.direcaoLinha = 0;
            jogador.direcaoColuna = 1;
            break;

        default:
            break;
    }
}


void moverJogador(void) {
    int novaLinha = jogador.linha + jogador.direcaoLinha;
    int novaColuna = jogador.coluna + jogador.direcaoColuna;

    if (novaLinha < 0 || novaLinha >= LINHAS ||
        novaColuna < 0 || novaColuna >= COLUNAS) {
        return;
    }

    if (mapa[novaLinha][novaColuna] == CAMINHO) {
        jogador.linha = novaLinha;
        jogador.coluna = novaColuna;
    }
}


void quebrarGelo(void) {
    int frenteLinha = jogador.linha + jogador.direcaoLinha;
    int frenteColuna = jogador.coluna + jogador.direcaoColuna;
    
    if (frenteLinha < 0 || frenteLinha >= LINHAS ||
        frenteColuna < 0 || frenteColuna >= COLUNAS) {
        return;
    }

    if (mapa[frenteLinha][frenteColuna] == PAREDE_GELO) {
        mapa[frenteLinha][frenteColuna] = CAMINHO;
    }
}

void reconstruirGelo(void) {
    int frenteLinha = jogador.linha + jogador.direcaoLinha;
    int frenteColuna = jogador.coluna + jogador.direcaoColuna;

    if (frenteLinha < 0 || frenteLinha >= LINHAS ||
        frenteColuna < 0 || frenteColuna >= COLUNAS) {
        return;
    }

    if (mapa[frenteLinha][frenteColuna] == CAMINHO) {
        mapa[frenteLinha][frenteColuna] = PAREDE_GELO;
    }
}

int main(void) {
    int tecla;

    screenInit(1);
    keyboardInit();

    inicializarMapa();
    desenharMapa();

    do {
        tecla = readch();

        atualizarDirecao(tecla);

        switch (tecla) {
            case 'w': case 'W':
            case 'a': case 'A':
            case 's': case 'S':
            case 'd': case 'D':
                moverJogador();
                break;

            case 'b': case 'B':
                quebrarGelo();
                break;

            case 'r': case 'R':
                reconstruirGelo();
                break;

            default:
                break;
        }

        if (tecla != 'q' && tecla != 'Q') {
            desenharMapa();
        }

    } while (tecla != 'q' && tecla != 'Q');

    keyboardDestroy();
    screenDestroy();

    return 0;
}
