#include "cprocessing.h"
#include "credit.h"
#include "game.h"     // To get the death count
#include "utils.h"    // For IsAreaClicked
#include <stdio.h>

static CP_Font victory_font;
static float vo_timer = 0.0f; // Timer to prevent accidental clicks

// Loads the victory font and resets the input delay timer.
void Victory_Init(void) {
    vo_timer = 0.0f;
    victory_font = CP_Font_Load("Assets/Exo2-Regular.ttf");
    if (victory_font == 0) {
        printf("ERROR: Victory font failed to load! Check Assets folder.\n");
    }
}

// Renders the victory screen, calculates star rating based on deaths, and handles menu return.
void Victory_Update(void) {
    CP_Graphics_ClearBackground(CP_Color_Create(10, 20, 10, 255)); // Dark green background

    float ww = (float)CP_System_GetWindowWidth();
    float wh = (float)CP_System_GetWindowHeight();
    float mouse_x = (float)CP_Input_GetMouseX();
    float mouse_y = (float)CP_Input_GetMouseY();

    if (victory_font != 0) {
        CP_Font_Set(victory_font);
    }
    CP_Settings_RectMode(CP_POSITION_CENTER);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);

    // 1. Draw "VICTORY" (Bright Green)
    CP_Settings_Fill(CP_Color_Create(0, 255, 0, 255));
    CP_Settings_TextSize(100.0f);
    CP_Font_DrawText("VICTORY!", ww / 2.0f, wh / 2.0f - 150.0f);

    // 2. Get Death Count and Determine Stars
    int deaths = Game_Get_Death_Count();
    const char* stars_text;
    char death_count_text[64];

    if (deaths == 0) {
        stars_text = "***";
        snprintf(death_count_text, sizeof(death_count_text), "Flawless! (0 Restarts)");
    }
    else if (deaths <= 2) {
        stars_text = "**";
        snprintf(death_count_text, sizeof(death_count_text), "Well Done! (%d Restarts)", deaths);
    }
    else {
        stars_text = "*";
        snprintf(death_count_text, sizeof(death_count_text), "A Hard-Fought Victory! (%d Restarts)", deaths);
    }

    // 3. Draw Stars (Bright Yellow)
    CP_Settings_Fill(CP_Color_Create(255, 255, 0, 255));
    CP_Settings_TextSize(80.0f);
    CP_Font_DrawText(stars_text, ww / 2.0f, wh / 2.0f - 50.0f);

    // 4. Draw Death Count Text (White)
    CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255));
    CP_Settings_TextSize(24.0f);
    CP_Font_DrawText(death_count_text, ww / 2.0f, wh / 2.0f + 20.0f);


    // 5. Draw "Return to Menu" Button
    float menu_btn_x = ww / 2.0f;
    float menu_btn_y = wh / 2.0f + 120.0f;
    float btn_w = 300.0f;
    float btn_h = 70.0f;

    bool hover_menu = IsAreaClicked(menu_btn_x, menu_btn_y, btn_w, btn_h, mouse_x, mouse_y);

    if (hover_menu) CP_Settings_Fill(CP_Color_Create(150, 150, 150, 255));
    else CP_Settings_Fill(CP_Color_Create(100, 100, 100, 255));
    CP_Graphics_DrawRect(menu_btn_x, menu_btn_y, btn_w, btn_h);

    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Settings_TextSize(30.0f);
    CP_Font_DrawText("Go to Credits", menu_btn_x, menu_btn_y);

    // 6. Handle Input
    vo_timer += CP_System_GetDt();
    if (vo_timer > 0.5f && CP_Input_MouseClicked()) {
        if (hover_menu) {
            // move to credits
            CP_Engine_SetNextGameState(Credits_Init, Credits_Update, Credits_Exit);
        }
    }
}

// Frees the font used in the victory screen.
void Victory_Exit(void) {
    if (victory_font) {
        CP_Font_Free(victory_font);
    }
}