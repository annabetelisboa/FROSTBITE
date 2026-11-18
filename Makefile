# ============================================================
#  FROSTBITE - Makefile (C + Raylib)
#
#  Windows (MSYS2/MinGW):   mingw32-make
#  Linux/macOS:             make   (precisa da raylib 6.2 instalada; ver README)
#
#  Comandos:
#    mingw32-make              compila (build de debug)
#    mingw32-make run          compila e executa o jogo
#    mingw32-make release      rebuild otimizado (-O2, sem simbolos)
#    mingw32-make clean        remove build/ e o executavel
#    mingw32-make rebuild      clean + all
# ============================================================

CC      := gcc
TARGET  := frostbite

SRC_DIR := src
INC_DIR := include
LIB_DIR := lib
OBJ_DIR := build/obj

# Fontes do jogo. O tempCodeRunnerFile.c e um fragmento gerado pelo
# Code Runner (nao tem main) e por isso fica de fora da compilacao.
SRC := $(filter-out $(SRC_DIR)/tempCodeRunnerFile.c,$(wildcard $(SRC_DIR)/*.c))
OBJ := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

CFLAGS := -std=c11 -Wall -Wextra -I$(INC_DIR)

ifeq ($(BUILD),release)
  CFLAGS  += -O2
  LDFLAGS += -s
else
  CFLAGS  += -g
endif

# --- Diferencas entre Windows e Unix ---
ifeq ($(OS),Windows_NT)
  # No Windows o make usa o cmd.exe, mesmo quando chamado do Git Bash/MSYS2
  SHELL  := $(COMSPEC)
  EXE    := .exe
  MKDIR   = if not exist "$(subst /,\,$(1))" mkdir "$(subst /,\,$(1))"
  RMDIR   = if exist "$(subst /,\,$(1))" rmdir /S /Q "$(subst /,\,$(1))"
  DEL     = if exist "$(subst /,\,$(1))" del /Q "$(subst /,\,$(1))"
  RUN     = $(TARGET)$(EXE)
  # raylib estatica que vem em lib/ + bibliotecas do sistema do Windows
  LDLIBS := -L$(LIB_DIR) -lraylib -lopengl32 -lgdi32 -lwinmm
else
  EXE    :=
  MKDIR   = mkdir -p "$(1)"
  RMDIR   = rm -rf "$(1)"
  DEL     = rm -f "$(1)"
  RUN     = ./$(TARGET)$(EXE)
  # A lib/ tem a raylib do Windows, entao aqui usamos a raylib instalada
  # no sistema (sudo make install da raylib 6.2, como no README).
  ifeq ($(shell uname -s),Darwin)
    LDLIBS := -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo
  else
    LDLIBS := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
  endif
endif

# ---------- Regras ----------

all: $(TARGET)$(EXE)

$(TARGET)$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	@$(call MKDIR,$(OBJ_DIR))

run: all
	@$(RUN)

clean:
	@$(call RMDIR,$(OBJ_DIR))
	@$(call DEL,$(TARGET)$(EXE))

rebuild: clean all

# Garante build limpo ao trocar de configuracao (make so olha datas)
release:
	@$(MAKE) clean
	@$(MAKE) BUILD=release

.PHONY: all run clean rebuild release
