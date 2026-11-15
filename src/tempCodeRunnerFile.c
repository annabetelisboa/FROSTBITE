    ToggleFullscreen();
    jogo.estado = MENU;
    jogo.pontos = 0;
    jogo.fase = 1;

    SetTargetFPS(60);

    while (!WindowShouldClose() && jogo.estado != SAIR) {
        if (IsKeyPressed(KEY_F11) || (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_ENTER))) {
            ToggleFullscreen();
        }
        switch (jogo.estado) {
            case MENU:
                telaMenu();
                break;
            case JOGANDO:
      