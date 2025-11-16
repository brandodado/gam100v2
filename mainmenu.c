#include "mainmenu.h"
#include "game.h"
#include "cprocessing.h"
#include "utils.h"
#include "tutorial.h"

#define BUTTON_WIDTH 300.0f
#define BUTTON_HEIGHT 80.0f
#define BUTTON_SPACING 40.0f

// Button positions (we will set these in Init)
static float button_start_x, button_start_y;
static float button_tutorial_x, button_tutorial_y;
static float button_quit_x, button_quit_y;

static CP_Font menu_font;

void Main_Menu_Init(void)
{
    menu_font = CP_Font_Load("Assets/Exo2-Regular.ttf");
    CP_Font_Set(menu_font);
    CP_Settings_TextSize(32);

    // Center-align text for drawing on buttons
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);

    // Calculate button positions
    float center_x = CP_System_GetWindowWidth() / 2.0f;
    float center_y = CP_System_GetWindowHeight() / 2.0f;

    // Stack buttons vertically
    button_start_y = center_y - BUTTON_HEIGHT - BUTTON_SPACING; // Top button
    button_tutorial_y = center_y;                              // Middle button
    button_quit_y = center_y + BUTTON_HEIGHT + BUTTON_SPACING; // Bottom button

    button_start_x = center_x;
    button_tutorial_x = center_x;
    button_quit_x = center_x;
}

/**
 * \brief Helper function to draw a single button
 */
static void DrawButton(const char* text, float x, float y, float w, float h, int is_hovered)
{
    CP_Settings_RectMode(CP_POSITION_CENTER);

    // Draw button rectangle
    if (is_hovered) {
        CP_Settings_Fill(CP_Color_Create(150, 150, 150, 255)); // Brighter when hovered
    }
    else {
        CP_Settings_Fill(CP_Color_Create(80, 80, 80, 255)); // Dark gray
    }
    CP_Graphics_DrawRect(x, y, w, h);

    // Draw button text
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255)); // White text
    CP_Font_DrawText(text, x, y + 2); // Small offset for better vertical centering
}

void Main_Menu_Update(void)
{
    CP_Graphics_ClearBackground(CP_Color_Create(20, 25, 28, 255));

    // Get mouse position
    float mouse_x = (float)CP_Input_GetMouseX();
    float mouse_y = (float)CP_Input_GetMouseY();

    // --- Check for hover state ---
    // Check if mouse is over any button
    int hover_start = IsAreaClicked(button_start_x, button_start_y, BUTTON_WIDTH, BUTTON_HEIGHT, mouse_x, mouse_y);
    int hover_tutorial = IsAreaClicked(button_tutorial_x, button_tutorial_y, BUTTON_WIDTH, BUTTON_HEIGHT, mouse_x, mouse_y);
    int hover_quit = IsAreaClicked(button_quit_x, button_quit_y, BUTTON_WIDTH, BUTTON_HEIGHT, mouse_x, mouse_y);

    // --- Draw buttons ---
    DrawButton("START GAME", button_start_x, button_start_y, BUTTON_WIDTH, BUTTON_HEIGHT, hover_start);
    DrawButton("TUTORIAL", button_tutorial_x, button_tutorial_y, BUTTON_WIDTH, BUTTON_HEIGHT, hover_tutorial);
    DrawButton("QUIT", button_quit_x, button_quit_y, BUTTON_WIDTH, BUTTON_HEIGHT, hover_quit);

    // --- Check for clicks ---
    if (CP_Input_MouseClicked())
    {
        if (hover_start) {
            // 1. Start Game
            CP_Engine_SetNextGameState(Game_Init, Game_Update, Game_Exit);
        }
        else if (hover_tutorial) {
            // 2. Go to Tutorial
            // This call now works because we included "tutorial.h"
            CP_Engine_SetNextGameState(Tutorial_Init, Tutorial_Update, Tutorial_Exit);
        }
        else if (hover_quit) {
            // 3. Exit Menu (Quit Game)
            CP_Engine_Terminate();
        }
    }

    // You can also keep the ESC key as a quick-quit option
    if (CP_Input_KeyTriggered(KEY_ESCAPE)) {
        CP_Engine_Terminate();
    }
}

void Main_Menu_Exit(void) {}