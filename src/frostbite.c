#include "raylib.h"

#define LINHAS 15
#define COLUNAS 30
#define TAMANHO_CELULA 30 // Tamanho em pixels de cada quadrado do mapa

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

void inicializarMapa(void) {
    for (int linha = 0; linha < LINHAS; linha++) {
        for (int coluna = 0; coluna < COLUNAS; coluna++) {
            if (linha == 0 || linha == LINHAS - 1 || coluna == 0 || coluna == COLUNAS - 1) {
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

void moverJogador(void) {
    int novaLinha = jogador.linha + jogador.direcaoLinha;
    int novaColuna = jogador.coluna + jogador.direcaoColuna;

    if (novaLinha >= 0 && novaLinha < LINHAS && novaColuna >= 0 && novaColuna < COLUNAS) {
        if (mapa[novaLinha][novaColuna] == CAMINHO) {
            jogador.linha = novaLinha;
            jogador.coluna = novaColuna;
        }
    }
}

void quebrarGelo(void) {
    int frenteLinha = jogador.linha + jogador.direcaoLinha;
    int frenteColuna = jogador.coluna + jogador.direcaoColuna;

    if (frenteLinha >= 0 && frenteLinha < LINHAS && frenteColuna >= 0 && frenteColuna < COLUNAS) {
        if (mapa[frenteLinha][frenteColuna] == PAREDE_GELO) {
            mapa[frenteLinha][frenteColuna] = CAMINHO;
        }
    }
}

void reconstruirGelo(void) {
    int frenteLinha = jogador.linha + jogador.direcaoLinha;
    int frenteColuna = jogador.coluna + jogador.direcaoColuna;

    if (frenteLinha >= 0 && frenteLinha < LINHAS && frenteColuna >= 0 && frenteColuna < COLUNAS) {
        if (mapa[frenteLinha][frenteColuna] == CAMINHO) {
            mapa[frenteLinha][frenteColuna] = PAREDE_GELO;
        }
    }
}

int main(void) {
    // Largura: 30 colunas * 30px = 900px | Altura: 15 linhas * 30px + 60px de HUD = 510px
    InitWindow(COLUNAS * TAMANHO_CELULA, LINHAS * TAMANHO_CELULA + 60, "FROSTBITE");
    SetTargetFPS(60);

    inicializarMapa();

    while (!WindowShouldClose()) {
        // --- ENTRADA E DIREÇÕES ---
        if (IsKeyPressed(KEY_W)) { jogador.direcaoLinha = -1; jogador.direcaoColuna = 0; moverJogador(); }
        if (IsKeyPressed(KEY_S)) { jogador.direcaoLinha = 1;  jogador.direcaoColuna = 0; moverJogador(); }
        if (IsKeyPressed(KEY_A)) { jogador.direcaoLinha = 0;  jogador.direcaoColuna = -1; moverJogador(); }
        if (IsKeyPressed(KEY_D)) { jogador.direcaoLinha = 0;  jogador.direcaoColuna = 1;  moverJogador(); }

        if (IsKeyPressed(KEY_B)) quebrarGelo();
        if (IsKeyPressed(KEY_R)) reconstruirGelo();

        // --- DESENHO DA TELA ---
        BeginDrawing();
            ClearBackground(DARKBLUE);

            // Desenhar o mapa
            for (int l = 0; l < LINHAS; l++) {
                for (int c = 0; c < COLUNAS; c++) {
                    int x = c * TAMANHO_CELULA;
                    int y = l * TAMANHO_CELULA;

                    if (mapa[l][c] == PAREDE_NORMAL) {
                        DrawRectangle(x, y, TAMANHO_CELULA - 1, TAMANHO_CELULA - 1, GRAY);
                    } else if (mapa[l][c] == PAREDE_GELO) {
                        DrawRectangle(x, y, TAMANHO_CELULA - 1, TAMANHO_CELULA - 1, SKYBLUE);
                    }
                }
            }

            // Desenhar Jogador
            DrawRectangle(jogador.coluna * TAMANHO_CELULA, jogador.linha * TAMANHO_CELULA, TAMANHO_CELULA - 1, TAMANHO_CELULA - 1, YELLOW);

            // HUD / Instruções na parte inferior
            DrawText("WASD: mover | B: quebrar gelo | R: reconstruir gelo", 15, LINHAS * TAMANHO_CELULA + 15, 18, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}