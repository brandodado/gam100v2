#include "cprocessing.h"
#include "mainmenu.h"
#include <stdio.h>

// Use a specific name to avoid conflicts with other files
static CP_Font gameover_font;
static float go_timer = 0.0f;

void GameOver_Init(void) {
    // 1. Reset Timer
    go_timer = 0.0f;

    // 2. Load Font (Use the exact filename you used in game.c)
    gameover_font = CP_Font_Load("Assets/Exo2-Regular.ttf");

    // Error Check: If font fails, print to console
    if (gameover_font == 0) {
        printf("ERROR: Game Over font failed to load! Check Assets folder.\n");
    }
}

void GameOver_Update(void) {
    // 1. Clear Background (Black)
    CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 255));

    float ww = (float)CP_System_GetWindowWidth();
    float wh = (float)CP_System_GetWindowHeight();

    // 2. SET FONT SETTINGS (Every Frame)
    // We do this here to ensure no other screen messed up the settings
    if (gameover_font != 0) {
        CP_Font_Set(gameover_font);
    }
    CP_Settings_RectMode(CP_POSITION_CENTER);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);

    // 3. Draw "GAME OVER" (Bright Red)
    CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
    CP_Settings_TextSize(100.0f);
    CP_Font_DrawText("GAME OVER", ww / 2.0f, wh / 2.0f - 50.0f);

    // 4. Draw Instruction (White)
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Settings_TextSize(30.0f);
    CP_Font_DrawText("Click to Return to Menu", ww / 2.0f, wh / 2.0f + 50.0f);

    // 5. Handle Input (Wait 0.5s before allowing click to prevent accidental skips)
    go_timer += CP_System_GetDt();

    if (go_timer > 0.5f && CP_Input_MouseClicked()) {
        CP_Engine_SetNextGameState(Main_Menu_Init, Main_Menu_Update, Main_Menu_Exit);
    }
}

void GameOver_Exit(void) {
    // Cleanup
    // CP_Font_Free(&gameover_font); // Optional depending on your engine version
}