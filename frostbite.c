#include "raylib.h"

int main(void) {
    InitWindow(800, 450, "Frostbite - Teste Raylib");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("funcionando porra", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}