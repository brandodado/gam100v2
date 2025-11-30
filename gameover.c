#include "cprocessing.h"
#include "mainmenu.h"
#include <stdio.h>
#include "game.h" 
#include "utils.h" 

// Use a specific name to avoid conflicts with other files
static CP_Font gameover_font;
static float go_timer = 0.0f;

// Loads resources and resets the timer for the Game Over screen.
void GameOver_Init(void) {
    // 1. Reset Timer
    go_timer = 0.0f;

    // 2. Load Font
    gameover_font = CP_Font_Load("Assets/Exo2-Regular.ttf");

    // Error Check: If font fails, print to console
    if (gameover_font == 0) {
        printf("ERROR: Game Over font failed to load! Check Assets folder.\n");
    }
}

// Renders the Game Over text and buttons, handling restarts from checkpoints.
void GameOver_Update(void) {
    // 1. Clear Background (Black)
    CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 255));

    float ww = (float)CP_System_GetWindowWidth();
    float wh = (float)CP_System_GetWindowHeight();
    float mouse_x = (float)CP_Input_GetMouseX();
    float mouse_y = (float)CP_Input_GetMouseY();

    // 2. SET FONT SETTINGS (Every Frame)
    if (gameover_font != 0) {
        CP_Font_Set(gameover_font);
    }
    CP_Settings_RectMode(CP_POSITION_CENTER);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);

    // 3. Draw "GAME OVER" (Bright Red)
    CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
    CP_Settings_TextSize(100.0f);
    CP_Font_DrawText("GAME OVER", ww / 2.0f, wh / 2.0f - 100.0f);

    // --- 4. MODIFIED: Draw Buttons ---
    CP_Settings_TextSize(30.0f);

    // Button positions
    float menu_btn_x = ww / 2.0f;
    float menu_btn_y = wh / 2.0f + 50.0f;
    float restart_btn_x = ww / 2.0f;
    float restart_btn_y = wh / 2.0f + 120.0f;
    float btn_w = 300.0f;
    float btn_h = 70.0f;

    // Check hover
    bool hover_menu = IsAreaClicked(menu_btn_x, menu_btn_y, btn_w, btn_h, mouse_x, mouse_y);
    bool hover_restart = IsAreaClicked(restart_btn_x, restart_btn_y, btn_w, btn_h, mouse_x, mouse_y);

    // Draw "Return to Menu" Button
    if (hover_menu) CP_Settings_Fill(CP_Color_Create(150, 150, 150, 255));
    else CP_Settings_Fill(CP_Color_Create(100, 100, 100, 255));
    CP_Graphics_DrawRect(menu_btn_x, menu_btn_y, btn_w, btn_h);

    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Font_DrawText("Return to Menu", menu_btn_x, menu_btn_y);

    // Draw "Restart Stage" Button
    if (hover_restart) CP_Settings_Fill(CP_Color_Create(150, 150, 150, 255));
    else CP_Settings_Fill(CP_Color_Create(100, 100, 100, 255));
    CP_Graphics_DrawRect(restart_btn_x, restart_btn_y, btn_w, btn_h);

    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Font_DrawText("Restart Stage", restart_btn_x, restart_btn_y);

    // 5. Handle Input (Wait 0.5s before allowing click to prevent accidental skips)
    go_timer += CP_System_GetDt();

    if (go_timer > 0.5f && CP_Input_MouseClicked()) {
        if (hover_menu) {
            CP_Engine_SetNextGameState(Main_Menu_Init, Main_Menu_Update, Main_Menu_Exit);
        }
        else if (hover_restart) {
            // --- MODIFIED: Set flag, increment counter, and call Game_Init ---
            Game_Set_Restart_Flag(true);
            Game_Increment_Death_Count();
            CP_Engine_SetNextGameState(Game_Init, Game_Update, Game_Exit);
        }
    }
}

// Frees the font resource for the Game Over screen.
void GameOver_Exit(void) {
    // Cleanup
    if (gameover_font) {
        CP_Font_Free(gameover_font);
    }
}