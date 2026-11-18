#include "estrutura.h"
#include "mapa.h"

char mapa[LINHAS][COLUNAS];

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

    // Área inicial do jogador
    mapa[1][1] = CAMINHO;
    mapa[1][2] = CAMINHO;
    mapa[1][3] = CAMINHO;
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

void preencherMapaBase(void) {
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

void inicializarMapaFase2(void) {
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
