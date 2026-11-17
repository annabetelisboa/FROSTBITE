#ifndef INIMIGOS_H
#define INIMIGOS_H

#include "estrutura.h"

extern Inimigo inimigos[MAX_INIMIGOS];

int existeInimigo(int linha, int coluna);
int calcularDistancia(Posicao a, Posicao b);
void definirRota(int indice, const Posicao rota[], int tamanho);
void inicializarInimigos(void);
void prepararRotasInimigos(void);
Posicao escolherMovimentoInteligente(Posicao opcoes[], int quantidade, int indiceInimigo);
int posicaoNoHistorico(Inimigo *inimigo, Posicao posicao);
void moverInimigos(void);
int buscarProximoPasso(Posicao origem, Posicao destino, Posicao *proximoPasso);
void moverPerseguidor(void);
int jogadorAtingido(void);

#endif
