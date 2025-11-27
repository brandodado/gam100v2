#include "cprocessing.h"
#include "intro.h"
#include "mainmenu.h"
#include <stdio.h>

// --- Configuration ---
#define FADE_SPEED 200.0f     // Speed of fade in/out
#define HOLD_DURATION 2.0f    // How long to stay fully visible

// --- Assets ---
static CP_Image logo_digipen = NULL;
static CP_Image logo_game = NULL;
static CP_Font copyright_font = NULL;

// --- State Variables ---
static float alpha = 0.0f;    // Current transparency (0-255)
static float timer = 0.0f;    // Timer for holding the image
static int current_phase = 0; // 0 = Fade In DP, 1 = Hold DP, 2 = Fade Out DP, 3 = Fade In Game, 4 = Hold Game, 5 = Fade Out Game

void Intro_Init(void)
{
    // Load Assets
    // Make sure these files exist in your Assets folder!
    logo_digipen = CP_Image_Load("Assets/DigiPen_Logo.png");
    logo_game = CP_Image_Load("Assets/HexHand_Logo.png"); // Your game logo

    copyright_font = CP_Font_Load("Assets/Exo2-Regular.ttf"); // Using same font as game

    // Reset State
    alpha = 0.0f;
    timer = 0.0f;
    current_phase = 0;

    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_BASELINE);
}

void Intro_Update(void)
{
    float dt = CP_System_GetDt();
    float width = (float)CP_System_GetWindowWidth();
    float height = (float)CP_System_GetWindowHeight();

    // Always Black Background
    CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 255));

    // --- LOGIC STATE MACHINE ---
    switch (current_phase) {

        // 1. DIGIPEN LOGO: FADE IN
    case 0:
        alpha += FADE_SPEED * dt;
        if (alpha >= 255.0f) {
            alpha = 255.0f;
            current_phase = 1; // Move to Hold
        }
        break;

        // 2. DIGIPEN LOGO: HOLD
    case 1:
        timer += dt;
        if (timer >= HOLD_DURATION) {
            timer = 0.0f;
            current_phase = 2; // Move to Fade Out
        }
        break;

        // 3. DIGIPEN LOGO: FADE OUT
    case 2:
        alpha -= FADE_SPEED * dt;
        if (alpha <= 0.0f) {
            alpha = 0.0f;
            current_phase = 3; // Move to Game Logo
        }
        break;

        // 4. GAME LOGO: FADE IN
    case 3:
        alpha += FADE_SPEED * dt;
        if (alpha >= 255.0f) {
            alpha = 255.0f;
            current_phase = 4; // Move to Hold
        }
        break;

        // 5. GAME LOGO: HOLD
    case 4:
        timer += dt;
        if (timer >= HOLD_DURATION) {
            timer = 0.0f;
            current_phase = 5; // Move to Fade Out
        }
        break;

        // 6. GAME LOGO: FADE OUT
    case 5:
        alpha -= FADE_SPEED * dt;
        if (alpha <= 0.0f) {
            alpha = 0.0f;
            // --- TRANSITION TO MAIN MENU ---
            CP_Engine_SetNextGameState(Main_Menu_Init, Main_Menu_Update, Main_Menu_Exit);
        }
        break;
    }

    // --- RENDER ---

    // Drawing DigiPen Screen (Phases 0, 1, 2)
    if (current_phase <= 2) {
        if (logo_digipen) {
            // Draw Logo Centered
            float w = (float)CP_Image_GetWidth(logo_digipen);
            float h = (float)CP_Image_GetHeight(logo_digipen);
            // Optional: Scale down if too big
            if (w > width * 0.8f) {
                float scale = (width * 0.8f) / w;
                w *= scale;
                h *= scale;
            }

            CP_Image_Draw(logo_digipen, width / 2.0f, height / 2.0f, w, h, (int)alpha);
        }

        // Draw Copyright Notice
        if (copyright_font) CP_Font_Set(copyright_font);
        CP_Settings_Fill(CP_Color_Create(255, 255, 255, (int)alpha));
        CP_Settings_TextSize(14.0f);
        CP_Font_DrawText("All content (c) 2025 DigiPen Institute of Technology Singapore. All Rights Reserved.", width / 2.0f, height - 50.0f);
    }
    // Drawing Game Logo Screen (Phases 3, 4, 5)
    else {
        if (logo_game) {
            float w = (float)CP_Image_GetWidth(logo_game);
            float h = (float)CP_Image_GetHeight(logo_game);
            CP_Image_Draw(logo_game, width / 2.0f, height / 2.0f, w, h, (int)alpha);
        }
        else {
            // Fallback if no logo image found: Draw Text
            if (copyright_font) CP_Font_Set(copyright_font);
            CP_Settings_Fill(CP_Color_Create(255, 255, 255, (int)alpha));
            CP_Settings_TextSize(80.0f);
            CP_Font_DrawText("HEXHAND", width / 2.0f, height / 2.0f);
        }
    }

    // Allow skipping with Space or Click
    if (CP_Input_KeyTriggered(KEY_SPACE) || CP_Input_MouseClicked()) {
        CP_Engine_SetNextGameState(Main_Menu_Init, Main_Menu_Update, Main_Menu_Exit);
    }
}

void Intro_Exit(void)
{
    // Clean up assets
    CP_Image_Free(logo_digipen);
    CP_Image_Free(logo_game);
}