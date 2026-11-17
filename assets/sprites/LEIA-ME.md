# Sprites do FROSTBITE

Todos os desenhos do jogo ficam nesta pasta, em PNG com fundo transparente
(quando precisa). O jogo carrega esses arquivos **em tempo de execução** —
depois de editar, é só fechar e abrir o jogo de novo. **Não precisa recompilar.**

## Arquivos e formatos (não mude o tamanho do canvas!)

| Arquivo      | Tamanho | Conteúdo |
|--------------|---------|----------|
| `chao.png`   | 20×20   | 1 tile de chão (neve/gelo fino) |
| `parede.png` | 20×20   | 1 tile de parede (pedra congelada) |
| `gelo.png`   | 80×20   | **4 quadros lado a lado** (20×20 cada) |
| `pinguim.png`| 160×20  | **8 quadros lado a lado** (20×20 cada) |
| `foca.png`   | 160×20  | **8 quadros** — inimigo patrulhador |
| `urso.png`   | 160×20  | **8 quadros** — inimigo perseguidor |
| `peixe.png`  | 80×20   | **4 quadros** — item coletável (no lugar da fruta) |
| `fundo.png`  | 1200×660| fundo das telas (menu, recordes, ajustes, game over) |
| `fim.png`    | 1200×660| fundo da tela final, após a última fase |

Cada tile de 20×20 é desenhado no jogo em 2× (40×40 px).

### Ordem dos quadros do `gelo.png`

```
[ 0 ][ 1 ][ 2 ][ 3 ]
```

São 4 variações de rachadura. O jogo escolhe uma pela posição da célula,
então basta desenhar variações diferentes — a ordem não tem significado.

### Ordem dos quadros do `pinguim.png`

```
[ 0 ][ 1 ][ 2 ][ 3 ][ 4 ][ 5 ][ 6 ][ 7 ]
  ^^^^^^    ^^^^^^    ^^^^^^    ^^^^^^
   frente    costas   esquerda  direita
   parado/andando (0/1), (2/3), (4/5), (6/7)
```

| Índice | Direção  | Frame |
|--------|----------|-------|
| 0 | frente   | parado  |
| 1 | frente   | andando |
| 2 | costas   | parado  |
| 3 | costas   | andando |
| 4 | esquerda | parado  |
| 5 | esquerda | andando |
| 6 | direita  | parado  |
| 7 | direita  | andando |

O "parado" é usado quando o pinguim não está se movendo; o "andando" aparece
durante o passo.

## Inimigos: `foca.png` e `urso.png`

As duas folhas seguem **exatamente a mesma ordem** do pinguim
(8 quadros = 4 direções × 2 frames):

- `foca.png` → inimigo **patrulhador** (anda em rota fixa, laranja no jogo antigo)
- `urso.png` → inimigo **perseguidor** (usa BFS atrás do jogador, vermelho no jogo antigo)

```
[ 0 ][ 1 ][ 2 ][ 3 ][ 4 ][ 5 ][ 6 ][ 7 ]
 frente    costas    esquerda  direita
```

A direção é deduzida do movimento (para onde o bicho andou); quando ele para,
usa o quadro "parado".

## Item coletável: `peixe.png`

A fruta antiga virou peixe. A folha tem **4 quadros de animação** da cauda
(20×20 cada), tocados em loop enquanto o peixe está no mapa:

```
[ 0 ][ 1 ][ 2 ][ 3 ]
```

Não há significado de direção: são só os quadros da nadada. O jogo escolhe o
quadro pelo tempo e desloca a animação pela posição de cada peixe.

## Fundo das telas: `fundo.png`

Imagem única de **1200×660** usada no menu, recordes, ajustes, fase concluída
e game over. Se você trocar por outra, mantenha **1200×660**.

O desenho tem céu em gradiente, aurora boreal, lua, estrelas, montanhas de gelo,
**mar congelado com icebergs e banquisas**, colinas de neve, **pinheiros nevados**
e um **iglu** com luz acesa na porta, além de neve caindo.

- No game over o jogo aplica uma **tonalidade avermelhada** por cima.
- O jogo ainda escurece levemente o fundo para os textos ficarem legíveis.

## Como editar

1. Abra o PNG em um editor de pixel art com zoom (a imagem é minúscula):
   - **Piskel** (grátis, roda no navegador) — https://www.piskelapp.com
   - **LibreSprite** (grátis) — https://libresprite.github.io
   - **Aseprite** (pago) — https://www.aseprite.org
   - **Paint.NET** / **GIMP** também funcionam.
2. Salve **em cima do mesmo arquivo**, mantendo exatamente o mesmo tamanho.
3. Feche e abra o jogo (`mingw32-make run` ou `frostbite.exe`).

## Os PNGs agora são a única fonte da arte

Os sprites não são mais gerados por código: edite os PNGs desta pasta
diretamente. Se quiser voltar a ter um gerador, use o histórico do git.
