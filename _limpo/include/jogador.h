#ifndef JOGADOR_H
#define JOGADOR_H

#include "estrutura.h"

extern Jogador jogador;

void moverJogador(int direcaoLinha, int direcaoColuna);
void quebrarGelo();
void reconstruirGelo();

#endif
