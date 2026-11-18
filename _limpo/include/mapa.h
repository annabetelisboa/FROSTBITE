#ifndef MAPA_H
#define MAPA_H

#include "estrutura.h"

extern char mapa[LINHAS][COLUNAS];

void preencherMapaBase(void);
void abrirCorredor(int linha, int inicio, int fim);
void paredeHorizontal(int linha, int inicio, int fim, int abertura);
void inicializarMapaFase1(void);
void inicializarMapaFase2(void);
void inicializarMapaFase3(void);
int posicaoValida(int linha, int coluna);

#endif
