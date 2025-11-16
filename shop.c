#include "shop.h"
#include <stdio.h>      // For snprintf
#include "cprocessing.h"
#include "utils.h"      // For IsAreaClicked
#include "game.h"       // For game logic/transitions
#include "mainmenu.h"   // For Main Menu transition

#define CHOICE_BTN_W 400.0f // Increased width for text
#define CHOICE_BTN_H 100.0f

// --- Static Helper Function ---

static void DrawChoiceButton(const char* text, float x, float y, float w, float h, int is_hovered)
{
    CP_Settings_RectMode(CP_POSITION_CENTER);

    // Draw button rectangle
    if (is_hovered) {
        CP_Settings_Fill(CP_Color_Create(150, 150, 150, 255)); // Brighter
    }
    else {
        CP_Settings_Fill(CP_Color_Create(80, 80, 80, 255)); // Dark gray
    }

    CP_Settings_StrokeWeight(2.0f);
    CP_Settings_Stroke(CP_Color_Create(200, 200, 200, 255));
    CP_Graphics_DrawRect(x, y, w, h);

    // Draw button text
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));

    // Use the global game_font
    CP_Font_Set(game_font);

    CP_Settings_TextSize(24.0f);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Font_DrawText(text, x, y + 2.0f);
}

// --- Shop State Functions ---

void Shop_Init(void)
{
    // Set the global font as active
    if (game_font) {
        CP_Font_Set(game_font);
    }
}

void Shop_Update(void)
{
    float ww = (float)CP_System_GetWindowWidth();
    float wh = (float)CP_System_GetWindowHeight();

    // --- 1. Define choices based on level ---
    char textA[64], textB[64];

    int healthA = 0;
    float multiplierA = 0.0f;
    bool lifestealA = false;
    bool drawA = false;

    int healthB = 0;
    float multiplierB = 0.0f;
    bool lifestealB = false;
    bool drawB = false;


    // We read the global current_level from game.c
    if (current_level == 1) { // Player just cleared level 1
        snprintf(textA, 64, "Way of the Vampire (Lifesteal)");
        lifestealA = true;

        snprintf(textB, 64, "Way of the Titan (+50 Max Health)");
        healthB = 50;
    }
    else if (current_level == 2) { // Player just cleared level 2
        snprintf(textA, 64, "Focused Power (Card Effects x2.0)");
        multiplierA = 1.0f;

        snprintf(textB, 64, "Deeper Reserves (+30 HP, +1 Card Draw)");
        healthB = 30;
        drawB = true;
    }
    else {
        // Player has beaten Level 3. Go to Main Menu.
        // --- REMOVED: Checkpoint Flag Reset ---
        CP_Engine_SetNextGameState(Main_Menu_Init, Main_Menu_Update, Main_Menu_Exit);
        return; // Exit this function immediately
    }

    // --- 2. Draw the UI ---
    CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 255)); // Clear screen
    CP_Settings_Fill(CP_Color_Create(0, 0, 0, 200));
    CP_Settings_RectMode(CP_POSITION_CORNER);
    CP_Graphics_DrawRect(0, 0, ww, wh); // Draw semi-transparent overlay

    // Use the global game_font
    CP_Font_Set(game_font);

    CP_Settings_TextSize(48);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Settings_Fill(CP_Color_Create(255, 255, 0, 255));
    CP_Font_DrawText("CHOOSE YOUR REWARD", ww / 2.0f, wh / 2.0f - 150.0f);

    // Define button positions
    float buttonA_x = ww / 2.0f;
    float buttonA_y = wh / 2.0f;
    float buttonB_x = ww / 2.0f;
    float buttonB_y = wh / 2.0f + 120.0f;

    float mouse_x = (float)CP_Input_GetMouseX();
    float mouse_y = (float)CP_Input_GetMouseY();
    int mouse_clicked = CP_Input_MouseClicked();

    int hoverA = IsAreaClicked(buttonA_x, buttonA_y, CHOICE_BTN_W, CHOICE_BTN_H, mouse_x, mouse_y);
    int hoverB = IsAreaClicked(buttonB_x, buttonB_y, CHOICE_BTN_W, CHOICE_BTN_H, mouse_x, mouse_y);

    // Draw the buttons (using the static helper in this file)
    DrawChoiceButton(textA, buttonA_x, buttonA_y, CHOICE_BTN_W, CHOICE_BTN_H, hoverA);
    DrawChoiceButton(textB, buttonB_x, buttonB_y, CHOICE_BTN_W, CHOICE_BTN_H, hoverB);

    // --- 3. Check for Click ---
    if (mouse_clicked) {
        if (hoverA) {
            ExecuteLevelUpChoice(healthA, multiplierA, lifestealA, drawA);
        }
        else if (hoverB) {
            ExecuteLevelUpChoice(healthB, multiplierB, lifestealB, drawB);
        }

        if (hoverA || hoverB) {
            // 1. Manually advance the level
            current_level++;
            // 2. Load the new level
            LoadLevel(current_level);
            // 3. Transition back to the game (Update, no Init)
            CP_Engine_SetNextGameState(NULL, Game_Update, Game_Exit);
        }
    }
}

void Shop_Exit(void)
{
    // This function is now empty.
}