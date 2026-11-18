#include <stdlib.h>
#include <stdio.h>
#include "raylib.h"
#include "estrutura.h"
#include "jogador.h"
#include "inimigos.h"
#include "mapa.h"
#include "pontuacao.h"

// Textura onde o jogo é desenhado na resolução lógica (LARGURA_JOGO x ALTURA_JOGO).
// No fim do frame ela é desenhada na janela, escalada e centralizada.
RenderTexture2D alvo;

// Sprites do design (carregados em main, liberados no fim).
Texture2D spriteChao;
Texture2D spriteParede;
Texture2D spriteGelo;
Texture2D spritePinguim;
Texture2D spriteFoca;
Texture2D spriteUrso;
Texture2D spritePeixe;
Texture2D spriteFundo;
Texture2D spriteFim;
Texture2D logoMenu;

int opcaoMenu = 0;
double tempoUltimoMovimento = 0.0;
float volumeMusica = 0.5f;

// Procura um sprite em caminhos comuns, para funcionar tanto rodando
// da raiz do projeto quanto de dentro de src/.
static const char *caminhoSprite(const char *arquivo) {
    static char caminho[512];
    const char *raizes[] = {
        "assets/sprites",
        "../assets/sprites",
        "../../assets/sprites"
    };

    for (int i = 0; i < (int)(sizeof(raizes) / sizeof(raizes[0])); i++) {
        snprintf(caminho, sizeof(caminho), "%s/%s", raizes[i], arquivo);

        if (FileExists(caminho)) {
            return caminho;
        }
    }

    snprintf(caminho, sizeof(caminho), "assets/sprites/%s", arquivo);
    return caminho;
}

// Procura um arquivo na raiz do projeto (ou acima), para assets que nao ficam em assets/sprites.
static const char *caminhoProjeto(const char *arquivo) {
    static char caminho[512];
    const char *raizes[] = { "", "../", "../../" };

    for (int i = 0; i < (int)(sizeof(raizes) / sizeof(raizes[0])); i++) {
        snprintf(caminho, sizeof(caminho), "%s%s", raizes[i], arquivo);

        if (FileExists(caminho)) {
            return caminho;
        }
    }

    snprintf(caminho, sizeof(caminho), "%s", arquivo);
    return caminho;
}

// Desenha um sprite de 20x20 esticado para uma célula de 40x40.
static void desenharSpriteCelula(Texture2D textura, int quadro,
                                 int coluna, int linha, Color corFallback) {
    int x = coluna * TAMANHO_CELULA;
    int y = linha * TAMANHO_CELULA;

    if (IsTextureValid(textura)) {
        Rectangle origem = {
            (float)(quadro * TAMANHO_SPRITE), 0.0f,
            (float)TAMANHO_SPRITE, (float)TAMANHO_SPRITE
        };
        Rectangle destino = {
            (float)x, (float)y,
            (float)TAMANHO_CELULA, (float)TAMANHO_CELULA
        };

        DrawTexturePro(textura, origem, destino,
                       (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
    } else if (corFallback.a > 0) {
        DrawRectangle(x, y, TAMANHO_CELULA - 1, TAMANHO_CELULA - 1, corFallback);
    }
}

// Escolhe o sprite do pinguim pela última direção e anima quando ele anda.
static void desenharPinguim(void) {
    int x = jogador.coluna * TAMANHO_CELULA;
    int y = jogador.linha * TAMANHO_CELULA;

    // sombra no chão (dá profundidade ao cartoon)
    DrawEllipse(x + TAMANHO_CELULA / 2, y + 36, 11.0f, 4.0f, Fade(BLACK, 0.18f));

    if (!IsTextureValid(spritePinguim)) {
        DrawRectangle(x, y, TAMANHO_CELULA - 1, TAMANHO_CELULA - 1, YELLOW);
        return;
    }

    int direcao = 0; // 0 frente, 1 costas, 2 esquerda, 3 direita

    if (jogador.direcaoLinha < 0) {
        direcao = 1;
    } else if (jogador.direcaoLinha > 0) {
        direcao = 0;
    } else if (jogador.direcaoColuna < 0) {
        direcao = 2;
    } else if (jogador.direcaoColuna > 0) {
        direcao = 3;
    }

    int andando = (GetTime() - tempoUltimoMovimento) < 0.15;
    int quadro = andando ? ((int)(GetTime() * 8.0) % 2) : 0;
    int indice = direcao * 2 + quadro;

    Rectangle origem = {
        (float)(indice * TAMANHO_SPRITE), 0.0f,
        (float)TAMANHO_SPRITE, (float)TAMANHO_SPRITE
    };
    Rectangle destino = {
        (float)x, (float)y,
        (float)TAMANHO_CELULA, (float)TAMANHO_CELULA
    };

    DrawTexturePro(spritePinguim, origem, destino,
                   (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
}

// Desenha os inimigos: foca (patrulhadora) e urso-polar (perseguidor).
// A direcao do olhar e o ritmo do passo sao deduzidos da propria posicao,
// entao nenhuma regra do jogo muda por causa do desenho.
static void desenharInimigos(void) {
    static Posicao posicaoVista[MAX_INIMIGOS];
    static double instantePasso[MAX_INIMIGOS];
    static int direcaoVista[MAX_INIMIGOS];
    static int pronto = 0;

    if (!pronto) {
        for (int i = 0; i < MAX_INIMIGOS; i++) {
            posicaoVista[i] = inimigos[i].posicao;
            direcaoVista[i] = 0;
            instantePasso[i] = -10.0;
        }
        pronto = 1;
    }

    for (int i = 0; i < MAX_INIMIGOS; i++) {
        if (!inimigos[i].ativo) {
            continue;
        }

        int dl = inimigos[i].posicao.linha - posicaoVista[i].linha;
        int dc = inimigos[i].posicao.coluna - posicaoVista[i].coluna;

        if (dl != 0 || dc != 0) {
            // Passos de 1 celula viram direcao + animacao. Saltos grandes
            // (troca de fase) so reposicionam, sem trocar o olhar.
            if (abs(dl) <= 1 && abs(dc) <= 1) {
                if (dc != 0) {
                    direcaoVista[i] = (dc > 0) ? 3 : 2;
                } else {
                    direcaoVista[i] = (dl > 0) ? 0 : 1;
                }

                instantePasso[i] = GetTime();
            }

            posicaoVista[i] = inimigos[i].posicao;
        }

        int x = inimigos[i].posicao.coluna * TAMANHO_CELULA;
        int y = inimigos[i].posicao.linha * TAMANHO_CELULA;

        // sombra no chão (dá profundidade ao cartoon)
        DrawEllipse(x + TAMANHO_CELULA / 2, y + 36, 11.0f, 4.0f,
                    Fade(BLACK, 0.18f));

        Texture2D textura = inimigos[i].perseguidor ? spriteUrso : spriteFoca;

        if (!IsTextureValid(textura)) {
            DrawCircle(x + TAMANHO_CELULA / 2, y + TAMANHO_CELULA / 2,
                       inimigos[i].perseguidor ? 13 : 9,
                       inimigos[i].perseguidor ? RED : ORANGE);
            continue;
        }

        int andando = (GetTime() - instantePasso[i]) < 0.25;
        int quadro = andando ? ((int)(GetTime() * 8.0) % 2) : 0;
        int indice = direcaoVista[i] * 2 + quadro;

        Rectangle origem = {
            (float)(indice * TAMANHO_SPRITE), 0.0f,
            (float)TAMANHO_SPRITE, (float)TAMANHO_SPRITE
        };
        Rectangle destino = {
            (float)x, (float)y,
            (float)TAMANHO_CELULA, (float)TAMANHO_CELULA
        };

        DrawTexturePro(textura, origem, destino,
                       (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
    }
}

// Fundo das telas (menu, recordes, ajustes, fase concluída, game over).
// O "tinta" multiplica as cores: WHITE mantém o original.
static void desenharFundo(Color tinta) {
    ClearBackground(BLACK);

    if (IsTextureValid(spriteFundo)) {
        DrawTexture(spriteFundo, 0, 0, tinta);

        // Escurece de leve pra garantir leitura dos textos.
        DrawRectangle(0, 0, LARGURA_JOGO, ALTURA_JOGO, Fade(BLACK, 0.18f));
    } else {
        ClearBackground(DARKBLUE);
    }
}

// ---- HUD (barra fixa de informações no rodapé) ----

static const Color HUD_TOPO = { 18, 38, 58, 255 };
static const Color HUD_BASE = { 7, 18, 32, 255 };
static const Color HUD_GELO = { 124, 180, 214, 255 };
static const Color HUD_TEXTO = { 236, 247, 255, 255 };
static const Color HUD_TEXTO_FRACO = { 138, 174, 202, 255 };
static const Color HUD_BARRA_FUNDO = { 11, 26, 42, 255 };
static const Color HUD_BARRA_CHEIA = { 96, 214, 246, 255 };
static const Color HUD_BARRA_BRILHO = { 196, 242, 255, 255 };

// Texto alinhado à esquerda, com a mesma sombra usada nas outras telas.
static void desenharTextoSombra(const char *texto, int x, int y,
                                int tamanho, Color cor) {
    DrawText(texto, x + 2, y + 2, tamanho, Fade(BLACK, 0.55f));
    DrawText(texto, x, y, tamanho, cor);
}

// Progresso em blocos: um bloco por peixe da fase.
static void desenharBarraPeixes(int x, int y, int largura, int altura,
                                int coletados, int total) {
    if (total <= 0) {
        return;
    }

    int vao = 2;
    int bloco = (largura - (total - 1) * vao) / total;

    if (bloco < 2) {
        bloco = 2;
    }
    if (total > 20) {
        // Fases com muitos peixes: blocos finos demais viram ruído.
        vao = 1;
        bloco = (largura - (total - 1) * vao) / total;

        if (bloco < 2) {
            bloco = 2;
        }
    }

    for (int i = 0; i < total; i++) {
        int bx = x + i * (bloco + vao);
        Rectangle r = { (float)bx, (float)y, (float)bloco, (float)altura };

        if (i < coletados) {
            DrawRectangleRec(r, HUD_BARRA_CHEIA);
            DrawRectangle(bx, y, bloco, 3, HUD_BARRA_BRILHO);
        } else {
            DrawRectangleRec(r, HUD_BARRA_FUNDO);
            DrawRectangleLinesEx(r, 1.0f, Fade(HUD_GELO, 0.22f));
        }
    }
}

// Painel de gelo com peixes coletados, pontos, progresso, fase e controles.
static void desenharHUD(void) {
    int y0 = LINHAS * TAMANHO_CELULA;
    int coletados = contarFrutasColetadas();

    // Painel com gradiente, aro de gelo no topo e sombra embaixo.
    DrawRectangleGradientV(0, y0, LARGURA_JOGO, 60, HUD_TOPO, HUD_BASE);
    DrawRectangle(0, y0, LARGURA_JOGO, 2, Fade(HUD_GELO, 0.85f));
    DrawRectangle(0, y0 + 2, LARGURA_JOGO, 1, Fade(HUD_GELO, 0.25f));
    DrawRectangle(0, y0 + 57, LARGURA_JOGO, 3, Fade(BLACK, 0.55f));

    // Poeira de gelo no painel (posições fixas, sem piscar).
    for (int i = 0; i < 46; i++) {
        int px = (i * 197 + 13) % (LARGURA_JOGO - 16) + 8;
        int py = y0 + 6 + (i * 53) % 46;

        DrawPixel(px, py, Fade(WHITE, 0.06f + (float)(i % 4) * 0.03f));
    }

    // --- esquerda: peixe animado + coletados/total ---
    if (IsTextureValid(spritePeixe)) {
        int quadro = ((int)(GetTime() * 5.0)) % QUADROS_PEIXE;
        Rectangle origem = {
            (float)(quadro * TAMANHO_SPRITE), 0.0f,
            (float)TAMANHO_SPRITE, (float)TAMANHO_SPRITE
        };
        Rectangle destino = { 20.0f, (float)(y0 + 10), 40.0f, 40.0f };

        DrawCircle(40, y0 + 30, 21, Fade(BLACK, 0.20f));
        DrawTexturePro(spritePeixe, origem, destino,
                       (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
    }

    const char *textoColetados = TextFormat("%d", coletados);

    desenharTextoSombra(textoColetados, 72, y0 + 12, 28, HUD_TEXTO);
    desenharTextoSombra(TextFormat("/%d", quantidadeFrutasAtivas),
                        72 + MeasureText(textoColetados, 28) + 4, y0 + 20,
                        18, HUD_TEXTO_FRACO);

    DrawRectangle(200, y0 + 12, 1, 36, Fade(HUD_GELO, 0.22f));

    // --- meio: pontuação ---
    desenharTextoSombra("PONTOS", 224, y0 + 6, 10, HUD_TEXTO_FRACO);
    desenharTextoSombra(TextFormat("%06d", jogo.pontos), 224, y0 + 18, 28,
                        HUD_TEXTO);

    DrawRectangle(400, y0 + 12, 1, 36, Fade(HUD_GELO, 0.22f));

    // --- progresso em blocos ---
    desenharTextoSombra("PEIXES", 424, y0 + 6, 10, HUD_TEXTO_FRACO);
    desenharBarraPeixes(424, y0 + 25, 280, 12, coletados,
                        quantidadeFrutasAtivas);

    // --- direita: fase e controles ---
    const char *textoFase = TextFormat("FASE %d", jogo.fase);
    int badgeLargura = MeasureText(textoFase, 16) + 36;
    Rectangle badge = {
        (float)(LARGURA_JOGO - 16 - badgeLargura), (float)(y0 + 10),
        (float)badgeLargura, 26.0f
    };

    DrawRectangleRounded(badge, 0.45f, 4, Fade(HUD_GELO, 0.14f));
    DrawRectangleRoundedLinesEx(badge, 0.45f, 4, 1.0f,
                                Fade(HUD_GELO, 0.45f));
    desenharTextoSombra(textoFase,
                        (int)(badge.x +
                              (badge.width - MeasureText(textoFase, 16)) / 2),
                        y0 + 15, 16, HUD_TEXTO);

    const char *controles = "WASD mover  |  K gelo  |  P reconstruir";

    desenharTextoSombra(controles,
                        LARGURA_JOGO - 16 - MeasureText(controles, 13),
                        y0 + 41, 13, HUD_TEXTO_FRACO);
}

void desenharJogo(void) {
    BeginTextureMode(alvo);
    ClearBackground(DARKBLUE);

    // Chão de neve em todas as células.
    for (int l = 0; l < LINHAS; l++) {
        for (int c = 0; c < COLUNAS; c++) {
            desenharSpriteCelula(spriteChao, 0, c, l, BLANK);
        }
    }

    // Paredes de pedra e blocos de gelo.
    for (int l = 0; l < LINHAS; l++) {
        for (int c = 0; c < COLUNAS; c++) {
            if (mapa[l][c] == PAREDE_NORMAL) {
                desenharSpriteCelula(spriteParede, 0, c, l, GRAY);
            } else if (mapa[l][c] == PAREDE_GELO) {
                int variante = (l * 7 + c * 13) % QUADROS_GELO;
                desenharSpriteCelula(spriteGelo, variante, c, l, SKYBLUE);
            }
        }
    }

    // Peixes (itens coletáveis) ainda não coletados.
    for (int i = indiceInicialFrutas;
         i < indiceInicialFrutas + quantidadeFrutasAtivas;
         i++) {
        if (!frutaColetada[i]) {
            int x = frutas[i].coluna * TAMANHO_CELULA;
            int y = frutas[i].linha * TAMANHO_CELULA;

            if (IsTextureValid(spritePeixe)) {
                // Cada peixe nada com um pequeno atraso em relação ao vizinho.
                int quadro = ((int)(GetTime() * 6.0) +
                              frutas[i].coluna + frutas[i].linha) % QUADROS_PEIXE;

                DrawEllipse(x + TAMANHO_CELULA / 2, y + 35, 9.0f, 3.5f,
                            Fade(BLACK, 0.16f));

                desenharSpriteCelula(spritePeixe, quadro,
                                     frutas[i].coluna, frutas[i].linha, BLANK);
            } else {
                DrawCircle(
                    x + TAMANHO_CELULA / 2,
                    y + TAMANHO_CELULA / 2,
                    8,
                    RED
                );

                DrawCircle(x + 18, y + 9, 3, GREEN);
            }
        }
    }

    // Inimigos: foca (patrulha) e urso-polar (perseguidor).
    desenharInimigos();

    // Jogador (pinguim).
    desenharPinguim();

    // Barra fixa de informações no rodapé.
    desenharHUD();

    EndTextureMode();
}

// Desenha um texto centralizado na largura do jogo. Usa LARGURA_JOGO (e não
// a largura da janela) porque as telas são desenhadas na textura "alvo",
// que tem resolução fixa e só depois é escalada para a janela.
void desenharTextoCentro(const char *texto, int y, int tamanho, Color cor) {
    int x = (LARGURA_JOGO - MeasureText(texto, tamanho)) / 2;

    // Sombra curta: garante leitura sobre o fundo nevado.
    DrawText(texto, x + 2, y + 2, tamanho, Fade(BLACK, 0.55f));
    DrawText(texto, x, y, tamanho, cor);
}

// Título com sombra escura atrás: fica legível sobre o fundo nevado.
void desenharTitulo(const char *texto, int y, int tamanho, Color cor) {
    desenharTextoCentro(texto, y + 3, tamanho, Fade(BLACK, 0.55f));
    desenharTextoCentro(texto, y, tamanho, cor);
}

// Coloca o jogo no começo de uma fase (serve pra começar,
// reiniciar e passar de fase).
void iniciarFase(void) {
    jogador = (Jogador){1, 1, 0, 1};

    const Fase *config = obterFase(jogo.fase);

    if (config == NULL) {
        return;
    }

    indiceInicialFrutas = 0;
    segundaLevaFase2 = 0;
    terceiraLevaFase3 = 0;

    quantidadeFrutasAtivas = config->frutasPrimeiraLeva;

    for (int i = 0; i < MAX_FRUTAS; i++) {
        frutaColetada[i] = 0;
    }

    if (config->mapa == 1) {
        inicializarMapaFase1();
    } else if (config->mapa == 2) {
        inicializarMapaFase2();
    } else if (config->mapa == 3) {
        inicializarMapaFase3();
    }

    inicializarInimigos();
    prepararRotasInimigos();

    // Barreira inicial de gelo
    for (int c = 1; c < COLUNAS - 1; c++) {
        if (mapa[5][c] != PAREDE_NORMAL) {
            mapa[5][c] = PAREDE_GELO;
        }
    }
}

// Lógica de um frame do jogo (sem desenhar).
void atualizarJogo(void) {
    double intervaloMovimento = 0.13;

    if (GetTime() - tempoUltimoMovimento >= intervaloMovimento) {

        if (IsKeyDown(KEY_W)) {
            moverJogador(-1, 0);
            tempoUltimoMovimento = GetTime();
        }
        else if (IsKeyDown(KEY_S)) {
            moverJogador(1, 0);
            tempoUltimoMovimento = GetTime();
        }
        else if (IsKeyDown(KEY_A)) {
            moverJogador(0, -1);
            tempoUltimoMovimento = GetTime();
        }
        else if (IsKeyDown(KEY_D)) {
            moverJogador(0, 1);
            tempoUltimoMovimento = GetTime();
        }
    }

    if (IsKeyPressed(KEY_K)) quebrarGelo();
    if (IsKeyPressed(KEY_P)) reconstruirGelo();

    moverInimigos();
    moverPerseguidor();

    if (jogadorAtingido()) {
        inserirRecorde(jogo.pontos, jogo.fase);
        salvarRecordes();
        jogo.estado = GAME_OVER;
        return;
    }

    const Fase *config = obterFase(jogo.fase);

    if (config != NULL && contarFrutasColetadas() == quantidadeFrutasAtivas) {

        // Inicia a segunda leva.
        if (segundaLevaFase2 == 0 &&
            config->frutasSegundaLeva > 0) {

            segundaLevaFase2 = 1;

            indiceInicialFrutas = config->frutasPrimeiraLeva;
            quantidadeFrutasAtivas = config->frutasSegundaLeva;

        }
        // Inicia a terceira leva.
        else if (terceiraLevaFase3 == 0 &&
                 config->frutasTerceiraLeva > 0) {

            terceiraLevaFase3 = 1;

            indiceInicialFrutas =
                config->frutasPrimeiraLeva +
                config->frutasSegundaLeva;

            quantidadeFrutasAtivas = config->frutasTerceiraLeva;
        }
        // Todas as levas foram concluídas.
        else {
            jogo.estado = FASE_CONCLUIDA;
        }

        // Reinicia o estado das frutas da nova leva.
        if (jogo.estado != FASE_CONCLUIDA) {
            for (int i = indiceInicialFrutas;
                 i < indiceInicialFrutas + quantidadeFrutasAtivas;
                 i++) {
                frutaColetada[i] = 0;
            }
        }
    }
}

// Tela de fase concluída: ENTER vai pra próxima fase (ou pra tela final).
void telaFaseConcluida(void) {
    int ultimaFase = (obterFase(jogo.fase + 1) == NULL);

    if (IsKeyPressed(KEY_ENTER)) {
        if (ultimaFase) {
            // Zerou o jogo: a pontuação entra nos recordes, como no game over.
            inserirRecorde(jogo.pontos, jogo.fase);
            salvarRecordes();
            jogo.estado = FIM;
        } else {
            jogo.fase++;
            iniciarFase();          // carrega o mapa e os inimigos da nova fase
            jogo.estado = JOGANDO;
        }
    }

    BeginTextureMode(alvo);
    desenharFundo(WHITE);
    desenharTitulo("FASE CONCLUIDA!", 90, 35, GREEN);
    desenharTextoCentro(TextFormat("Pontos: %d", jogo.pontos), 160, 22, RAYWHITE);

    const Fase *faseAtual = obterFase(jogo.fase);
    int totalFase = (faseAtual != NULL) ? faseAtual->totalFrutas : quantidadeFrutasAtivas;
    desenharTextoCentro(TextFormat("Peixes: %d/%d", contarFrutasColetadasFase(totalFase), totalFase),
                        195, 22, RAYWHITE);
    desenharTextoCentro(ultimaFase ? "ENTER: ver final" : "ENTER: proxima fase",
                        270, 18, RAYWHITE);
    EndTextureMode();
}

// Tela final: aparece depois de completar a ultima fase.
void telaFim(void) {
    if (IsKeyPressed(KEY_ENTER)) {
        jogo.pontos = 0;
        jogo.fase = 1;
        iniciarFase();
        jogo.estado = MENU;
    }

    BeginTextureMode(alvo);
    ClearBackground(BLACK);

    if (IsTextureValid(spriteFim)) {
        DrawTexture(spriteFim, 0, 0, WHITE);
    } else {
        ClearBackground(DARKBLUE);
    }

    // Sem acentos: a fonte padrão da raylib só tem caracteres ASCII
    // (o mesmo padrão das outras telas, como "Voce foi alcancado").
    desenharTitulo("PARABENS!", 110, 44, GOLD);
    desenharTextoCentro("Voce completou todas as fases.", 180, 24, RAYWHITE);
    desenharTextoCentro(TextFormat("Pontos: %d", jogo.pontos), 220, 22, SKYBLUE);
    desenharTextoCentro("Em breve mais atualizacoes!", 270, 24, YELLOW);
    desenharTextoCentro("ENTER: voltar ao menu", 325, 18, RAYWHITE);
    EndTextureMode();
}

// Tela de game over: ENTER recomeça do zero.
void telaGameOver(void) {
    if (IsKeyPressed(KEY_ENTER)) {
        jogo.pontos = 0;
        jogo.fase = 1;
        iniciarFase();
        jogo.estado = MENU;
    }

    BeginTextureMode(alvo);
    desenharFundo((Color){ 255, 176, 176, 255 });
    desenharTitulo("FIM DE JOGO!", 90, 35, RED);
    desenharTextoCentro("Voce foi alcancado por um inimigo.", 140, 20, RAYWHITE);
    desenharTextoCentro(TextFormat("Pontos: %d", jogo.pontos), 190, 22, RAYWHITE);

    // Mesmo cálculo da tela de fase concluída: peixes da fase inteira
    // (todas as levas), e não só da leva em que o jogador perdeu.
    const Fase *faseAtual = obterFase(jogo.fase);
    int totalFase = (faseAtual != NULL) ? faseAtual->totalFrutas : quantidadeFrutasAtivas;
    desenharTextoCentro(TextFormat("Peixes: %d/%d", contarFrutasColetadasFase(totalFase), totalFase),
                        225, 22, RAYWHITE);
    desenharTextoCentro("ENTER: voltar ao menu", 290, 18, RAYWHITE);
    EndTextureMode();
}

void telaMenu(void) {
    const char *opcoes[NUM_OPCOES_MENU] = {"Jogar", "Recordes", "Ajustes", "Sair"};

    // Navegação com setas ou W/S
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        opcaoMenu = (opcaoMenu + 1) % NUM_OPCOES_MENU;
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        opcaoMenu = (opcaoMenu + NUM_OPCOES_MENU - 1) % NUM_OPCOES_MENU;
    }

    // Confirmação
    if (IsKeyPressed(KEY_ENTER)) {
        if (opcaoMenu == 0) {
            jogo.pontos = 0;
            jogo.fase = 1;
            iniciarFase();
            jogo.estado = JOGANDO;
        } else if (opcaoMenu == 1) {
            jogo.estado = RECORDES;
        } else if (opcaoMenu == 2) {
            jogo.estado = AJUSTES;
        } else {
            jogo.estado = SAIR;
        }
    }

    BeginTextureMode(alvo);
    desenharFundo(WHITE);

    // Painel escuro atrás das opções, pra destacar do fundo nevado.
    DrawRectangleRounded((Rectangle){ 300, 138, 600, 205 }, 0.08f, 6,
                         Fade(BLACK, 0.34f));

    if (IsTextureValid(logoMenu)) {
        float larguraLogo = 650.0f;
        float alturaLogo = larguraLogo * (float)logoMenu.height / logoMenu.width;
        DrawTexturePro(logoMenu,
                       (Rectangle){ 0, 0, (float)logoMenu.width, (float)logoMenu.height },
                       (Rectangle){ (LARGURA_JOGO - larguraLogo) / 2.0f, 42,
                                    larguraLogo, alturaLogo },
                       (Vector2){ 0, 0 }, 0, WHITE);
    } else {
        desenharTitulo("FROSTBITE", 50, 50, SKYBLUE);
    }

    for (int i = 0; i < NUM_OPCOES_MENU; i++) {
        if (i == opcaoMenu) {
            desenharTextoCentro(TextFormat("> %s <", opcoes[i]),
                                160 + i * 45, 28, YELLOW);
        } else {
            desenharTextoCentro(opcoes[i], 160 + i * 45, 28, RAYWHITE);
        }
    }

    desenharTextoCentro("Setas ou W/S: escolher | ENTER: confirmar",
                        ALTURA_JOGO - 40, 16, RAYWHITE);
    EndTextureMode();
}

// Ajusta o volume da música pelas setas esquerda e direita.
void telaAjustes(Music musica) {
    if (IsKeyPressed(KEY_LEFT) && volumeMusica > 0.0f) {
        volumeMusica -= 0.1f;
        if (volumeMusica < 0.05f) volumeMusica = 0.0f;
    }
    if (IsKeyPressed(KEY_RIGHT) && volumeMusica < 1.0f) {
        volumeMusica += 0.1f;
        if (volumeMusica > 0.95f) volumeMusica = 1.0f;
    }

    if (IsMusicValid(musica)) {
        SetMusicVolume(musica, volumeMusica);
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_BACKSPACE)) {
        jogo.estado = MENU;
    }

    BeginTextureMode(alvo);
    desenharFundo(WHITE);
    desenharTitulo("AJUSTES", 50, 40, GOLD);
    desenharTextoCentro("Volume da musica", 140, 26, RAYWHITE);
    desenharTextoCentro(TextFormat("%d%%", (int)(volumeMusica * 100.0f + 0.5f)),
                        180, 24, YELLOW);

    Rectangle barra = { 390.0f, 225.0f, 420.0f, 24.0f };
    DrawRectangleRounded(barra, 0.4f, 8, Fade(BLACK, 0.45f));
    if (volumeMusica > 0.0f) {
        Rectangle preenchimento = {
            barra.x, barra.y, barra.width * volumeMusica, barra.height
        };
        DrawRectangleRounded(preenchimento, 0.4f, 8, SKYBLUE);
    }

    desenharTextoCentro("SETA ESQUERDA: diminuir | SETA DIREITA: aumentar",
                        285, 18, RAYWHITE);
    desenharTextoCentro("0% desliga a musica", 315, 16, LIGHTGRAY);
    desenharTextoCentro("ENTER ou BACKSPACE: voltar ao menu",
                        ALTURA_JOGO - 40, 16, RAYWHITE);
    EndTextureMode();
}

// Mostra o top 5 lido da lista de recordes.
void telaRecordes(void) {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_BACKSPACE)) {
        jogo.estado = MENU;
    }

    BeginTextureMode(alvo);
    desenharFundo(WHITE);

    // Painel escuro atrás da lista, pra destacar do fundo nevado.
    DrawRectangleRounded((Rectangle){ 250, 120, 700, 215 }, 0.08f, 6,
                         Fade(BLACK, 0.34f));

    desenharTitulo("RECORDES", 50, 40, GOLD);

    if (recordes == NULL) {
        desenharTextoCentro("Nenhum recorde ainda.", 160, 22, RAYWHITE);
    } else {
        int pos = 0;
        for (Recorde *r = recordes; r != NULL && pos < 5; r = r->prox, pos++) {
            desenharTextoCentro(
                TextFormat("%d. %d pontos (fase %d)", pos + 1, r->pontos, r->fase),
                140 + pos * 40, 24, RAYWHITE
            );
        }
    }

    desenharTextoCentro("ENTER: voltar ao menu", ALTURA_JOGO - 40, 16, RAYWHITE);
    EndTextureMode();
}

// Procura a música em caminhos comuns, para funcionar tanto rodando
// da raiz do projeto quanto de dentro de src/.
static const char *caminhoDaMusica(void) {
    const char *opcoes[] = {
        "assets/musica.mp3",
        "../assets/musica.mp3",
        "src/musica.mp3"
    };

    for (int i = 0; i < (int)(sizeof(opcoes) / sizeof(opcoes[0])); i++) {
        if (FileExists(opcoes[i])) {
            return opcoes[i];
        }
    }

    return opcoes[0]; // não achou: o jogo segue sem som
}

// Desenha a textura do jogo centralizada na janela, mantendo a proporção.
void desenharTelaEscalada(void) {
    float escalaX = (float)GetScreenWidth() / LARGURA_JOGO;
    float escalaY = (float)GetScreenHeight() / ALTURA_JOGO;
    float escala = (escalaX < escalaY) ? escalaX : escalaY;

    float largura = LARGURA_JOGO * escala;
    float altura = ALTURA_JOGO * escala;
    float x = (GetScreenWidth() - largura) / 2.0f;
    float y = (GetScreenHeight() - altura) / 2.0f;

    DrawTexturePro(
        alvo.texture,
        (Rectangle){ 0.0f, 0.0f, (float)alvo.texture.width, -(float)alvo.texture.height },
        (Rectangle){ x, y, largura, altura },
        (Vector2){ 0.0f, 0.0f },
        0.0f,
        WHITE
    );
}

int main(void) {
    InitWindow(LARGURA_JOGO, ALTURA_JOGO, "FROSTBITE");
    ToggleFullscreen();

    // O jogo é desenhado nessa textura (resolução fixa) e escalado para a tela.
    // Filtro POINT mantém os pixels nítidos (sem borrar o pixel art).
    alvo = LoadRenderTexture(LARGURA_JOGO, ALTURA_JOGO);
    SetTextureFilter(alvo.texture, TEXTURE_FILTER_POINT);

    // Sprites do design (pixel art). Se faltar algum, o desenho cai no
    // retângulo colorido antigo (não quebra o jogo).
    spriteChao = LoadTexture(caminhoSprite("chao.png"));
    spriteParede = LoadTexture(caminhoSprite("parede.png"));
    spriteGelo = LoadTexture(caminhoSprite("gelo.png"));
    spritePinguim = LoadTexture(caminhoSprite("pinguim.png"));
    spriteFoca = LoadTexture(caminhoSprite("foca.png"));
    spriteUrso = LoadTexture(caminhoSprite("urso.png"));
    spritePeixe = LoadTexture(caminhoSprite("peixe.png"));
    spriteFundo = LoadTexture(caminhoSprite("fundo.png"));
    spriteFim = LoadTexture(caminhoSprite("fim.png"));
    logoMenu = LoadTexture(caminhoProjeto("frostbite-logo.png"));

    Texture2D sprites[] = {
        spriteChao, spriteParede, spriteGelo, spritePinguim,
        spriteFoca, spriteUrso, spritePeixe, spriteFundo, spriteFim
    };

    for (int i = 0; i < (int)(sizeof(sprites) / sizeof(sprites[0])); i++) {
        if (IsTextureValid(sprites[i])) {
            SetTextureFilter(sprites[i], TEXTURE_FILTER_POINT);
        }
    }

    jogo.estado = MENU;
    jogo.pontos = 0;
    jogo.fase = 1;
    carregarRecordes();

    SetTargetFPS(60);

    InitAudioDevice(); // Liga o sistema de som
    Music musica = LoadMusicStream(caminhoDaMusica());
    if (IsMusicValid(musica)) {
        SetMusicVolume(musica, volumeMusica);
        PlayMusicStream(musica);
    } else {
        TraceLog(LOG_WARNING, "MUSICA: nao foi possivel carregar assets/musica.mp3");
    }

    while (!WindowShouldClose() && jogo.estado != SAIR) {
        if (IsKeyPressed(KEY_F11) || (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_ENTER))) {
            ToggleFullscreen();
        }

        if (IsMusicValid(musica)) {
            UpdateMusicStream(musica); // sem isso a música nunca avança

            if (!IsMusicStreamPlaying(musica)) { // terminou: começa de novo
                SeekMusicStream(musica, 0.0f);
                PlayMusicStream(musica);
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        switch (jogo.estado) {
            case MENU:
                telaMenu();
                break;
            case JOGANDO:
                atualizarJogo();
                desenharJogo();
                break;
            case FASE_CONCLUIDA:
                telaFaseConcluida();
                break;
            case GAME_OVER:
                telaGameOver();
                break;
            case FIM:
                telaFim();
                break;
            case AJUSTES:
                telaAjustes(musica);
                break;
            case RECORDES:
                telaRecordes();
                break;
            default:
                jogo.estado = SAIR;
                break;
        }

        desenharTelaEscalada();
        EndDrawing();
    }

    if (IsMusicValid(musica)) {
        UnloadMusicStream(musica);
    }
    CloseAudioDevice();
    UnloadTexture(spriteChao);
    UnloadTexture(spriteParede);
    UnloadTexture(spriteGelo);
    UnloadTexture(spritePinguim);
    UnloadTexture(spriteFoca);
    UnloadTexture(spriteUrso);
    UnloadTexture(spritePeixe);
    UnloadTexture(spriteFundo);
    UnloadTexture(spriteFim);
    if (IsTextureValid(logoMenu)) UnloadTexture(logoMenu);
    UnloadRenderTexture(alvo);
    liberarRecordes();
    CloseWindow();
    return 0;
}
