![Logo](frostbite-logo.png)

#### Frostbite é um jogo arcade em que os jogadores exploram labirintos congelados, coletam itens, desviam de inimigos e usam o gelo para bloquear caminhos e criar estratégias. Ao avançar cada labirinto os inimigos aumentam e a dificuldade também. O jogo propõe  dinamismo e uma atmosfera congelante, os jogadores precisam superar obstáculos, escapar dos inimigos e encontrar o caminho para a vitória!

<br>

<div align="center">

# ❄️ como abrir o jogo ❄️

˚ ༘ ⋆｡˚ 🐧 ˚｡⋆ ༘ ˚

*um guia pra você entrar no labirinto congelado*

</div>

<br>

## 🧊 jeitinho rápido (sem instalar nada)

o jogo já vem prontinho no repositório 

1. clique no botão verde **`<> Code`** aqui no GitHub e depois em **Download ZIP**
2. descompacte a pasta
3. dê dois cliques no **`frostbite.exe`** 🐧

> 💙 o `frostbite.exe` precisa ficar dentro da pasta do projeto, junto com a pasta `assets`. é lá que moram os sprites e a música!

<br>

## ❄️ jeitinho de dev (compilando o código)

perfeito pra quem quer mexer no código ou jogar a versão mais nova 🛠️

### ✦ passo 1 · instale as ferramentas (só na primeira vez)

abra o **PowerShell** e cole:

```powershell
winget install --id Git.Git -e
winget install --id BrechtSanders.WinLibs.POSIX.UCRT -e
```

Depois **feche e abra o terminal de novo** pra ele reconhecer as ferramentas novas  🔄

quer conferir se deu tudo certo? cole:

```powershell
gcc --version
mingw32-make --version
```

se as duas mostrarem uma versão, tá tudo pronto 🎀

### ✦ passo 2 · baixe o jogo

```powershell
git clone https://github.com/annabetelisboa/FROSTBITE.git
cd FROSTBITE
```

### ✦ passo 3 · compile e jogue 🐟

```powershell
mingw32-make run
```

e pronto, o labirinto abre em tela cheia ❄️

<br>

<details>
<summary>🧸 <b>outros comandos úteis</b></summary>

<br>

| comando | o que faz |
|:--|:--|
| `mingw32-make` | só compila |
| `mingw32-make run` | compila e abre o jogo |
| `mingw32-make rebuild` | apaga a compilação antiga e compila do zero |
| `mingw32-make release` | versão otimizada e mais levinha |
| `mingw32-make clean` | apaga os arquivos compilados |

</details>

<details>
<summary>🩵 <b>deu algum probleminha?</b></summary>

<br>

- **"mingw32-make não é reconhecido"** → feche e abra o terminal de novo. se continuar, reinstale o passo 1
- **o jogo abriu sem imagens ou sem música** → rode o jogo de dentro da pasta `FROSTBITE`, que é onde fica a pasta `assets`
- **o botão ▶ / F5 do VS Code dá erro** → use o terminal com `mingw32-make run`, que é ele quem sabe onde está a raylib
- **o jogo só funciona no Windows** por enquanto, porque a raylib que vem no projeto é a versão pra Windows

</details>

<br>

## 🎮 controles

<div align="center">

| tecla | ação |
|:--:|:--|
| <kbd>W</kbd> <kbd>A</kbd> <kbd>S</kbd> <kbd>D</kbd> | mover o pinguim 🐧 |
| <kbd>K</kbd> | quebrar o gelo na direção que você está olhando 🧊 |
| <kbd>P</kbd> | reconstruir o gelo pra bloquear os inimigos ❄️ |
| <kbd>↑</kbd> <kbd>↓</kbd> ou <kbd>W</kbd> <kbd>S</kbd> | escolher opção no menu |
| <kbd>Enter</kbd> | confirmar |
| <kbd>←</kbd> <kbd>→</kbd> | volume da música, na tela de ajustes 🎵 |
| <kbd>F11</kbd> ou <kbd>Alt</kbd> + <kbd>Enter</kbd> | tela cheia / janela |
| <kbd>Esc</kbd> | sair do jogo |

</div>

<br>

## 🐟 como jogar

- colete **todos os peixes** de cada leva pra liberar a próxima 🐟
- cuidado com a **foca** 🦭, que patrulha o labirinto, e com o **urso-polar** 🐻‍❄️, que vem atrás de você!
- use o gelo a seu favor: quebre pra abrir caminho e reconstrua pra fechar a passagem dos inimigos
- são **3 fases**, cada uma mais difícil que a anterior. sobreviva até o final ✨

<br>

<div align="center">

˚ ༘ ⋆｡˚ ❄️ boa sorte no labirinto congelado ❄️ ˚｡⋆ ༘ ˚

</div>

<br>

## Ferramentas utilizadas:
* C
* Raylib


| Nome | Email |
|:-----|------:|
| Anna Elizabete | aeals@cesar.school |
| Bruna Rocha | br@cesar.school |
| Gabriela Manzi | gmsca@cesar.school |
