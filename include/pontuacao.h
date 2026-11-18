#ifndef PONTUACAO_H
#define PONTUACAO_H

#include "estrutura.h"

#define ARQUIVO_RECORDE "recordes.txt"
#define MAX_RECORDES_SALVOS 10 // o arquivo guarda só os melhores

extern Recorde *recordes;
extern Posicao frutas[MAX_FRUTAS];
extern int frutaColetada[MAX_FRUTAS];
extern int indiceInicialFrutas;
extern int quantidadeFrutasAtivas;
extern int segundaLevaFase2;
extern int terceiraLevaFase3;

int existeFruta(int linha, int coluna);
void coletarFruta(void);
int contarFrutasColetadas(void);
int contarFrutasColetadasFase(int total);
void inserirRecorde(int pontos, int fase);
void liberarRecordes(void);
void carregarRecordes(void);
void salvarRecordes(void);

#endif
