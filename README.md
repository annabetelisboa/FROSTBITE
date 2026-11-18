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
- **tá no linux?** → os passos acima são pro Windows. pule pra seção [🐧 jogando no linux](#-jogando-no-linux) logo abaixo 💙

</details>

<br>

## 🐧 jogando no linux

<div align="center">

˚₊‧꒰ა ☆ ໒꒱ ‧₊˚

*sim, o pinguim também mora no linux!* 🐧💙

</div>

> 🌷 o `Makefile` do projeto foi feito pro Windows, então no linux a gente compila com um comandinho só nosso. é rapidinho, prometo!

### ✦ passo 1 · ferramentas e bibliotecas (só na primeira vez)

escolha a sua distro e cole no terminal 🎀

**🍥 ubuntu / debian / mint**

```bash
sudo apt update
sudo apt install build-essential git libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
```

**🍡 fedora**

```bash
sudo dnf install gcc make git alsa-lib-devel mesa-libGL-devel libX11-devel libXrandr-devel libXi-devel libXcursor-devel libXinerama-devel libatomic
```

**🍙 arch / manjaro**

```bash
sudo pacman -S base-devel git alsa-lib mesa libx11 libxrandr libxi libxcursor libxinerama
```

### ✦ passo 2 · instale a raylib 6.2 (só na primeira vez)

o jogo usa a raylib **6.2**, então a gente instala exatamente essa versãozinha 🌸

```bash
git clone --depth 1 --branch 6.2 https://github.com/raysan5/raylib.git ~/raylib-6.2
cd ~/raylib-6.2/src
make PLATFORM=PLATFORM_DESKTOP
sudo make install
cd ~
```

> 💭 *por que não instalar pela loja de pacotes?* as versões de lá podem ser mais antigas que a 6.2, e aí o jogo não compila. assim fica tudo certinho ✨

### ✦ passo 3 · baixe o jogo

```bash
git clone https://github.com/annabetelisboa/FROSTBITE.git
cd FROSTBITE
```

### ✦ passo 4 · compile e jogue 🐟

```bash
gcc -std=c11 src/*.c -Iinclude -o frostbite -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
./frostbite
```

e pronto, o labirinto congelado abre no seu linux ❄️🐧

nas próximas vezes é só rodar `./frostbite` de dentro da pasta `FROSTBITE` 🧸
(se alguém mudar o código, rode o comando do `gcc` de novo pra atualizar)

<details>
<summary>🩵 <b>deu algum probleminha no linux?</b></summary>

<br>

- **`raylib.h: No such file or directory` ou `cannot find -lraylib`** → a raylib não foi instalada. refaça o passo 2 e veja se o `sudo make install` terminou sem erro
- **erro com `X11/Xlib.h` ou `GL/gl.h` no passo 2** → faltou alguma biblioteca do passo 1. cole o comando da sua distro de novo
- **o jogo abriu sem imagens ou sem música** → rode o `./frostbite` de dentro da pasta `FROSTBITE`, que é onde fica a pasta `assets`
- **o jogo abriu mas sem som** → confira se o som do sistema está funcionando. o jogo continua normalzinho mesmo sem áudio
- **testado no ubuntu 24.04** 🍥 em outras distros os passos são os mesmos, só muda o comando do passo 1

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
