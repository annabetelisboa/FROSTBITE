#include <stddef.h>
#include "estrutura.h"

Fase fases[] = {
    {1, 9, 9, 0, 0, 1, 12, 24, 1, 1},
    {2, 22, 13, 9, 0, 2, 10, 20, 1, 2},
    {3, 27, 9, 13, 5, 3, 8, 16, 2, 3}
};

Jogo jogo;

// manipulação das fases
const Fase *obterFase(int numero) {
    int quantidadeFases = sizeof(fases) / sizeof(fases[0]);

    for (int i = 0; i < quantidadeFases; i++) {
        if (fases[i].numero == numero) {
            return &fases[i];
        }
    }

    return NULL;
}
