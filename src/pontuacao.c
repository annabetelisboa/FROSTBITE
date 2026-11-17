#include <stdio.h>
#include <stdlib.h>
#include "estrutura.h"
#include "jogador.h"
#include "pontuacao.h"

Recorde *recordes = NULL;

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
