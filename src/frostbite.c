
#include "raylib.h"

#define LINHAS 15
#define COLUNAS 30
#define TAMANHO_CELULA 30
#define VELOCIDADE_INIMIGO 12

#define PAREDE_NORMAL '#'
#define PAREDE_GELO 'G'
#define CAMINHO ' '

#define MAX_FRUTAS 9
#define MAX_INIMIGOS 4
#define VELOCIDADE_INIMIGO 12
#define VELOCIDADE_PERSEGUIDOR 24
#define MAX_ROTA 24

typedef struct {
    int linha;
    int coluna;
} Posicao;

typedef struct {
    int linha;
    int coluna;
    int direcaoLinha;
    int direcaoColuna;
} Jogador;

typedef struct {
    Posicao posicao;
    Posicao rota[MAX_ROTA];
    int tamanhoRota;
    int indiceRota;
    int contador;
    int perseguidor;
    int sentido; // 1 = avança na rota; -1 = volta
} Inimigo;

char mapa[LINHAS][COLUNAS];
Jogador jogador = {1, 1, 0, 1};

Posicao frutas[MAX_FRUTAS] = {
    {1, 5}, {2, 12}, {4, 3},
    {4, 17}, {6, 24}, {8, 10},
    {10, 5}, {12, 19}, {13, 26}
};

int frutaColetada[MAX_FRUTAS] = {0};

Inimigo inimigos[MAX_INIMIGOS];

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

void inicializarMapa(void) {
    // Primeiro, preenche o interior inteiro com gelo.
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

    // Barreira inicial de gelo entre o jogador e os inimigos.
    for (int c = 1; c < COLUNAS - 1; c++) {
        if (mapa[5][c] != PAREDE_NORMAL) {
            mapa[5][c] = PAREDE_GELO;
        }
    }
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
        if (inimigos[i].posicao.linha == linha &&
            inimigos[i].posicao.coluna == coluna) {
            return 1;
        }
    }

    return 0;
}

void coletarFruta(void) {
    for (int i = 0; i < MAX_FRUTAS; i++) {
        if (!frutaColetada[i] &&
            frutas[i].linha == jogador.linha &&
            frutas[i].coluna == jogador.coluna) {
            frutaColetada[i] = 1;
        }
    }
}

int contarFrutasColetadas(void) {
    int total = 0;

    for (int i = 0; i < MAX_FRUTAS; i++) {
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

void quebrarGelo(void) {
    int dl = jogador.direcaoLinha;
    int dc = jogador.direcaoColuna;

    // O jogador precisa ter uma direção definida.
    if (dl == 0 && dc == 0) {
        return;
    }

    int l = jogador.linha + dl;
    int c = jogador.coluna + dc;

    while (posicaoValida(l, c)) {
        // A parede normal encerra a quebra.
        if (mapa[l][c] == PAREDE_NORMAL) {
            break;
        }

        // Quebra todos os blocos de gelo encontrados.
        if (mapa[l][c] == PAREDE_GELO) {
            mapa[l][c] = CAMINHO;
        }

        l += dl;
        c += dc;
    }
}


void reconstruirGelo(void) {
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

        // Não coloca gelo sobre personagens ou frutas.
        if (mapa[l][c] == CAMINHO &&
            !existeFruta(l, c) &&
            !existeInimigo(l, c)) {
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
    inimigos[indice].perseguidor = 0;
}

void inicializarInimigos(void) {
    // Inimigo 1: percorre a parte superior do mapa.
    Posicao rota1[] = {
        {2,2}, {2,3}, {2,4}, {2,5},
        {2,6}, {2,7}, {2,8}, {2,7},
        {2,6}, {2,5}, {2,4}, {2,3}
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

    inimigos[3].posicao = (Posicao){13, 20};
    mapa[13][20] = CAMINHO;
    inimigos[3].perseguidor = 1;
    inimigos[3].contador = -300;
    inimigos[3].tamanhoRota = 0;
    inimigos[3].indiceRota = 0;
}

void prepararRotasInimigos(void) {
    for (int i = 0; i < MAX_INIMIGOS; i++) {
        for (int j = 0; j < inimigos[i].tamanhoRota; j++) {
            int l = inimigos[i].rota[j].linha;
            int c = inimigos[i].rota[j].coluna;

            if (mapa[l][c] == PAREDE_GELO) {
                mapa[l][c] = CAMINHO;
            }
        }
    }
}

void moverInimigos(void) {
    int dl[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    for (int i = 0; i < MAX_INIMIGOS; i++) {
        if (inimigos[i].perseguidor) {
            continue;
        }

        inimigos[i].contador++;

        if (inimigos[i].contador < VELOCIDADE_INIMIGO) {
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

        // Escolhe um dos movimentos possíveis.
        if (quantidade > 0) {
            int escolha = GetRandomValue(0, quantidade - 1);
            inimigos[i].posicao = opcoes[escolha];
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
        if (!inimigos[i].perseguidor) {
            continue;
        }

        inimigos[i].contador++;

        if (inimigos[i].contador < VELOCIDADE_PERSEGUIDOR) {
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
    for (int i = 0; i < MAX_FRUTAS; i++) {
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
        int x = inimigos[i].posicao.coluna * TAMANHO_CELULA;
        int y = inimigos[i].posicao.linha * TAMANHO_CELULA;

        if (inimigos[i].perseguidor) {
            DrawCircle(
                x + TAMANHO_CELULA / 2,
                y + TAMANHO_CELULA / 2,
                13,
                RED
            );
        } else {
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
                   contarFrutasColetadas(), MAX_FRUTAS),
        15, LINHAS * TAMANHO_CELULA + 8,
        18, RAYWHITE
    );

    DrawText(
        "WASD: mover | B: quebrar gelo | R: reconstruir",
        15, LINHAS * TAMANHO_CELULA + 32,
        15, RAYWHITE
    );

    EndDrawing();
}

int main(void) {
    InitWindow(
        COLUNAS * TAMANHO_CELULA,
        LINHAS * TAMANHO_CELULA + 60,
        "FROSTBITE"
    );

    SetTargetFPS(60);

    inicializarMapa();
    inicializarInimigos();
    prepararRotasInimigos();

    // Barreira inicial de gelo
    for (int c = 1; c < COLUNAS - 1; c++) {
        if (mapa[5][c] != PAREDE_NORMAL) {
            mapa[5][c] = PAREDE_GELO;
        }
    }

    moverInimigos();
    moverPerseguidor();

    int venceu = 0;
    int perdeu = 0;

    while (!WindowShouldClose() && !venceu && !perdeu) {
        if (IsKeyPressed(KEY_W)) moverJogador(-1, 0);
        if (IsKeyPressed(KEY_S)) moverJogador(1, 0);
        if (IsKeyPressed(KEY_A)) moverJogador(0, -1);
        if (IsKeyPressed(KEY_D)) moverJogador(0, 1);

        if (IsKeyPressed(KEY_B)) quebrarGelo();
        if (IsKeyPressed(KEY_R)) reconstruirGelo();

        moverInimigos();

        if (jogadorAtingido()) {
            perdeu = 1;
            break;
        }

        if (contarFrutasColetadas() == MAX_FRUTAS) {
            venceu = 1;
            break;
        }

        desenharJogo();
    }

    // Tela final.
    if (venceu || perdeu) {
        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(DARKBLUE);

            if (venceu) {
                DrawText(
                    "FASE CONCLUIDA!",
                    280, 190, 35, GREEN
                );
                DrawText(
                    "Voce coletou todas as frutas!",
                    240, 245, 20, RAYWHITE
                );
            } else {
                DrawText(
                    "FIM DE JOGO!",
                    320, 190, 35, RED
                );
                DrawText(
                    "Voce foi alcancado por um inimigo.",
                    215, 245, 20, RAYWHITE
                );
            }

            DrawText(
                "Feche a janela para sair.",
                300, 300, 18, RAYWHITE
            );

            EndDrawing();
        }
    }

    CloseWindow();
    return 0;
}
