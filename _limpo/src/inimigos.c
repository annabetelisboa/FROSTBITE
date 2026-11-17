#include <stdlib.h>
#include "raylib.h"
#include "estrutura.h"
#include "jogador.h"
#include "inimigos.h"
#include "mapa.h"

Inimigo inimigos[MAX_INIMIGOS];

int calcularDistancia(Posicao a, Posicao b) {
    int diferencaLinha = abs(a.linha - b.linha);
    int diferencaColuna = abs(a.coluna - b.coluna);

    return diferencaLinha + diferencaColuna;
}

int existeInimigo(int linha, int coluna) {
    for (int i = 0; i < MAX_INIMIGOS; i++) {
        if (!inimigos[i].ativo) {
            continue;
        }
        if (inimigos[i].posicao.linha == linha &&
            inimigos[i].posicao.coluna == coluna) {
            return 1;
        }
    }

    return 0;
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
    inimigos[indice].posicaoAnterior = rota[0];
    inimigos[indice].perseguidor = 0;
}

void inicializarInimigos(void) {
    for (int i = 0; i < MAX_INIMIGOS; i++) {
        inimigos[i].ativo = 0;
    }

    const Fase *config = obterFase(jogo.fase);

    if (config == NULL) {
        return;
    }

    // Inimigo 1: patrulha a região abaixo da barreira de gelo.
    Posicao rota1[] = {
        {6,2}, {6,3}, {6,4}, {6,5},
        {6,4}, {6,3}
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

    inimigos[0].ativo = 1;
    inimigos[1].ativo = 1;
    inimigos[2].ativo = 1;

    for (int i = 0; i < 3; i++) {
        inimigos[i].velocidade = config->velocidadeInimigos;
    }

    inimigos[3].posicao = (Posicao){13, 20};
    inimigos[3].posicaoAnterior = inimigos[3].posicao;
    inimigos[3].velocidade = config->velocidadePerseguidores;
    mapa[13][20] = CAMINHO;
    inimigos[3].perseguidor = 1;
    inimigos[3].contador = -300;
    inimigos[3].tamanhoRota = 0;
    inimigos[3].indiceRota = 0;
    inimigos[3].ativo = 1;

    if (jogo.fase == 3) {
        inimigos[4].posicao = (Posicao){1, 25};
        inimigos[4].posicaoAnterior = inimigos[4].posicao;
        inimigos[4].velocidade = config->velocidadePerseguidores;
        inimigos[4].perseguidor = 1;
        inimigos[4].contador = -450;
        inimigos[4].tamanhoRota = 0;
        inimigos[4].indiceRota = 0;
        inimigos[4].ativo = 1;

        mapa[1][25] = CAMINHO;
    }

    // Inicializa a memória de movimento dos inimigos.
    for (int i = 0; i < MAX_INIMIGOS; i++) {
        inimigos[i].quantidadeHistorico = 0;
        inimigos[i].indiceHistorico = 0;

        if (inimigos[i].ativo) {
            inimigos[i].historico[0] = inimigos[i].posicao;
            inimigos[i].quantidadeHistorico = 1;
        }
    }
}

void prepararRotasInimigos(void) {
    for (int i = 0; i < MAX_INIMIGOS; i++) {
        if (!inimigos[i].ativo) {
            continue;
        }
        for (int j = 0; j < inimigos[i].tamanhoRota; j++) {
            int l = inimigos[i].rota[j].linha;
            int c = inimigos[i].rota[j].coluna;

            if (mapa[l][c] == PAREDE_GELO) {
                mapa[l][c] = CAMINHO;
            }
        }
    }
}

Posicao escolherMovimentoInteligente(Posicao opcoes[], int quantidade, int indiceInimigo) {
    Posicao jogadorPos = {jogador.linha, jogador.coluna};

    Posicao melhor;
    int menorDistancia = LINHAS * COLUNAS + 1;
    int encontrou = 0;

    for (int i = 0; i < quantidade; i++) {

        int ehPosicaoAnterior =
            opcoes[i].linha == inimigos[indiceInimigo].posicaoAnterior.linha &&
            opcoes[i].coluna == inimigos[indiceInimigo].posicaoAnterior.coluna;

        // Evita voltar à posição anterior se houver outra opção.
        if (quantidade > 1 && ehPosicaoAnterior) {
            continue;
        }

        int distancia = calcularDistancia(opcoes[i], jogadorPos);

        if (!encontrou || distancia < menorDistancia) {
            menorDistancia = distancia;
            melhor = opcoes[i];
            encontrou = 1;
        }
    }

    if (!encontrou) {
        return opcoes[0];
    }

    return melhor;
}

int posicaoNoHistorico(Inimigo *inimigo, Posicao posicao) {
    for (int i = 0; i < inimigo->quantidadeHistorico; i++) {
        if (inimigo->historico[i].linha == posicao.linha &&
            inimigo->historico[i].coluna == posicao.coluna) {
            return 1;
        }
    }

    return 0;
}

void moverInimigos(void) {
    int dl[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    for (int i = 0; i < MAX_INIMIGOS; i++) {
        if (!inimigos[i].ativo) {
            continue;
        }
        if (inimigos[i].perseguidor) {
            continue;
        }

        inimigos[i].contador++;

        if (inimigos[i].contador < inimigos[i].velocidade) {
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

        // Escolhe um movimento evitando posições visitadas recentemente.
        if (quantidade > 0) {
            Posicao opcoesNovas[4];
            int quantidadeNovas = 0;

            // Separa as posições que não estão no histórico.
            for (int j = 0; j < quantidade; j++) {
                if (!posicaoNoHistorico(&inimigos[i], opcoes[j])) {
                    opcoesNovas[quantidadeNovas++] = opcoes[j];
                }
            }

            // Se todas as opções já foram visitadas,
            // permite voltar a alguma posição para não ficar parado.
            Posicao destino;

            if (quantidadeNovas > 0) {
                int escolha = GetRandomValue(0, quantidadeNovas - 1);
                destino = opcoesNovas[escolha];
            } else {
                int escolha = GetRandomValue(0, quantidade - 1);
                destino = opcoes[escolha];
            }

            // Realiza o movimento.
            inimigos[i].posicaoAnterior = inimigos[i].posicao;
            inimigos[i].posicao = destino;

            // Registra a nova posição no histórico circular.
            inimigos[i].indiceHistorico =
                (inimigos[i].indiceHistorico + 1) % MEMORIA_MOVIMENTO;

            inimigos[i].historico[inimigos[i].indiceHistorico] = destino;

            if (inimigos[i].quantidadeHistorico < MEMORIA_MOVIMENTO) {
                inimigos[i].quantidadeHistorico++;
            }
        }
    }
}

int buscarProximoPasso(Posicao origem, Posicao destino, Posicao *proximoPasso) {
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
                if (!inimigos[i].ativo) {
                    continue;
                }
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
        if (!inimigos[i].ativo) {
            continue;
        }
        if (!inimigos[i].perseguidor) {
            continue;
        }

        inimigos[i].contador++;

        if (inimigos[i].contador < inimigos[i].velocidade) {
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
        if (!inimigos[i].ativo) {
            continue;
        }
        if (jogador.linha == inimigos[i].posicao.linha &&
            jogador.coluna == inimigos[i].posicao.coluna) {
            return 1;
        }
    }

    return 0;
}
