#include "estrutura.h"
#include "jogador.h"
#include "mapa.h"
#include "inimigos.h"
#include "pontuacao.h"

Jogador jogador = {1, 1, 0, 1};

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
