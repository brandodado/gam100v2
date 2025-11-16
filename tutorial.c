#include "tutorial.h"       
#include "cprocessing.h"    
#include "mainmenu.h"       
#include "utils.h"          // For IsAreaClicked
#include <string.h>         // For strcmp function

// --- Static variables for this state ---
static CP_Font tutorial_font;
static int tutorialPage;    // Tracks which page we are on

// --- Button Definitions ---
#define TUTE_BUTTON_W 120.0f
#define TUTE_BUTTON_H 50.0f

/**
 * \brief Helper function to draw a single button (for text)
 */
static void DrawTutorialButton(const char* text, float x, float y, float w, float h, int is_hovered, float fontSize)
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
    CP_Font_Set(tutorial_font);

    CP_Settings_TextSize(fontSize);

    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Font_DrawText(text, x, y + 2.0f);
}

/**
 * \brief Helper function to draw a key with a triangle symbol
 */
static void DrawKeyWithTriangle(float x_center, float y_center, float size, const char* direction)
{
    // 1. Draw the box
    CP_Settings_RectMode(CP_POSITION_CENTER);
    CP_Settings_Fill(CP_Color_Create(80, 80, 80, 255));
    CP_Settings_StrokeWeight(2.0f);
    CP_Settings_Stroke(CP_Color_Create(200, 200, 200, 255));
    CP_Graphics_DrawRect(x_center, y_center, size, size);

    // 2. Draw the triangle
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Settings_NoStroke(); // Triangles shouldn't have an outline

    // Define the triangle points based on a padding
    float padding = size * 0.3f; // 30% padding
    float left = x_center - size / 2.0f + padding;
    float right = x_center + size / 2.0f - padding;
    float top = y_center - size / 2.0f + padding;
    float bottom = y_center + size / 2.0f - padding;
    float mid_x = x_center;
    float mid_y = y_center;

    CP_Vector p1, p2, p3;

    if (strcmp(direction, "UP") == 0) {
        p1 = CP_Vector_Set(mid_x, top);
        p2 = CP_Vector_Set(left, bottom);
        p3 = CP_Vector_Set(right, bottom);
    }
    else if (strcmp(direction, "DOWN") == 0) {
        p1 = CP_Vector_Set(mid_x, bottom);
        p2 = CP_Vector_Set(left, top);
        p3 = CP_Vector_Set(right, top);
    }
    else if (strcmp(direction, "LEFT") == 0) {
        p1 = CP_Vector_Set(left, mid_y);
        p2 = CP_Vector_Set(right, top);
        p3 = CP_Vector_Set(right, bottom);
    }
    else { // "RIGHT"
        p1 = CP_Vector_Set(right, mid_y);
        p2 = CP_Vector_Set(left, top);
        p3 = CP_Vector_Set(left, bottom);
    }

    CP_Graphics_DrawTriangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
}


void Tutorial_Init(void)
{
    // Load the font for this state
    tutorial_font = CP_Font_Load("Assets/Exo2-Regular.ttf");
    CP_Font_Set(tutorial_font);

    tutorialPage = 1; // Always start on page 1
}

void Tutorial_Update(void)
{
    CP_Graphics_ClearBackground(CP_Color_Create(20, 25, 28, 255));

    // --- Get Mouse ---
    float mouse_x = (float)CP_Input_GetMouseX();
    float mouse_y = (float)CP_Input_GetMouseY();

    int left_mouse_clicked = CP_Input_MouseClicked();

    // --- Define Panel Dimensions (with 'f' for floats) ---
    float panel_w = 800.0f;
    float panel_h = 550.0f;
    float panel_x = CP_System_GetWindowWidth() / 2.0f;
    float panel_y = CP_System_GetWindowHeight() / 2.0f;
    float panel_top = panel_y - (panel_h / 2.0f);
    float panel_left = panel_x - (panel_w / 2.0f);
    float panel_bottom = panel_y + (panel_h / 2.0f);
    float panel_right = panel_x + (panel_w / 2.0f);

    // --- Draw Panel Background ---
    CP_Settings_RectMode(CP_POSITION_CENTER);
    CP_Settings_Fill(CP_Color_Create(30, 30, 30, 240));
    CP_Settings_StrokeWeight(2.0f);
    CP_Settings_Stroke(CP_Color_Create(200, 200, 200, 255));
    CP_Graphics_DrawRect(panel_x, panel_y, panel_w, panel_h);
    CP_Settings_NoStroke(); // Reset stroke

    // --- Draw Panel Title ---
    CP_Font_Set(tutorial_font);
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Settings_TextSize(36);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);
    float title_y = panel_top + 30.0f;
    CP_Font_DrawText("TUTORIAL", panel_x, title_y);

    // --- Text settings (common for instruction pages) ---
    CP_Settings_TextSize(24);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
    float text_start_x = panel_left + 50.0f;
    float text_start_y = panel_top + 100.0f;
    float line_height = 40.0f;
    float text_box_width = panel_w - 100.0f; // Panel width minus 50px padding

    // --- Multi-page Logic ---
    if (tutorialPage == 1)
    {
        // --- Using CP_Font_DrawTextBox ---
        const char* page1_text = "1. This is a card game.\n\n"
            "2. Click on a card in your hand to select it.\n\n"
            "3. Select an enemy to target (if applicable).\n\n"
            "4. Click the 'Use Card' button to play it.\n\n"
            "5. Defeat all enemies to win the stage.";

        CP_Font_DrawTextBox(page1_text, text_start_x, text_start_y, text_box_width);

        // --- Draw "Next" Button ---
        float next_btn_x = panel_right - 80.0f;
        float next_btn_y = panel_bottom - 45.0f;
        int hover_next = IsAreaClicked(next_btn_x, next_btn_y, TUTE_BUTTON_W, TUTE_BUTTON_H, mouse_x, mouse_y);

        DrawTutorialButton("Next", next_btn_x, next_btn_y, TUTE_BUTTON_W, TUTE_BUTTON_H, hover_next, 24.0f);

        // --- Check for click ---
        if (left_mouse_clicked && hover_next) {
            tutorialPage = 2; // Go to page 2
        }
    }
    else if (tutorialPage == 2)
    {
        // --- Using CP_Font_DrawTextBox ---
        const char* page2_text = "HOW TO USE THE SHOP:\n\n"
            "1. The shop is available between levels.\n\n"
            "2. Click on a card to buy it.\n\n"
            "3. You can also remove cards from your deck.\n\n"
            "4. Spend your gold wisely!";

        CP_Font_DrawTextBox(page2_text, text_start_x, text_start_y, text_box_width);

        // --- Draw "Back" Button ---
        float back_btn_x = panel_left + 80.0f;
        float back_btn_y = panel_bottom - 45.0f;
        int hover_back = IsAreaClicked(back_btn_x, back_btn_y, TUTE_BUTTON_W, TUTE_BUTTON_H, mouse_x, mouse_y);
        DrawTutorialButton("Back", back_btn_x, back_btn_y, TUTE_BUTTON_W, TUTE_BUTTON_H, hover_back, 24.0f);

        // --- Draw "Next" Button ---
        float next_btn_x = panel_right - 80.0f;
        float next_btn_y = panel_bottom - 45.0f;
        int hover_next = IsAreaClicked(next_btn_x, next_btn_y, TUTE_BUTTON_W, TUTE_BUTTON_H, mouse_x, mouse_y);
        DrawTutorialButton("Next", next_btn_x, next_btn_y, TUTE_BUTTON_W, TUTE_BUTTON_H, hover_next, 24.0f);

        // --- Check for click ---
        if (left_mouse_clicked) {
            if (hover_back) {
                tutorialPage = 1; // Go back to page 1
            }
            else if (hover_next) {
                tutorialPage = 3;
            }
        }
    }
    else if (tutorialPage == 3)
    {
        // --- Using CP_Font_DrawTextBox ---
        const char* page3_text = "FURTHER INSTRUCTIONS:\n\n"
            "A. Some cards are 'Attack' (Red).\n\n"
            "B. Some cards are 'Heal' (Green).\n\n"
            "C. Your deck recycles when empty.\n\n"
            "D. Good luck!";

        CP_Font_DrawTextBox(page3_text, text_start_x, text_start_y, text_box_width);

        // --- Draw "Back" Button ---
        float back_btn_x = panel_left + 80.0f;
        float back_btn_y = panel_bottom - 45.0f;
        int hover_back = IsAreaClicked(back_btn_x, back_btn_y, TUTE_BUTTON_W, TUTE_BUTTON_H, mouse_x, mouse_y);
        DrawTutorialButton("Back", back_btn_x, back_btn_y, TUTE_BUTTON_W, TUTE_BUTTON_H, hover_back, 24.0f);

        // --- Draw "Next" Button ---
        float next_btn_x = panel_right - 80.0f;
        float next_btn_y = panel_bottom - 45.0f;
        int hover_next = IsAreaClicked(next_btn_x, next_btn_y, TUTE_BUTTON_W, TUTE_BUTTON_H, mouse_x, mouse_y);
        DrawTutorialButton("Next", next_btn_x, next_btn_y, TUTE_BUTTON_W, TUTE_BUTTON_H, hover_next, 24.0f);

        // --- Check for click ---
        if (left_mouse_clicked) {
            if (hover_back) {
                tutorialPage = 2; // Go back to page 2 (Shop)
            }
            else if (hover_next) {
                tutorialPage = 4; // Go to page 4 (Basic Controls)
            }
        }
    }
    else if (tutorialPage == 4) // *** UPDATED PAGE 4 LAYOUT ***
    {
        // --- Define a common Y for the top of the diagrams ---
        float diagram_top_y = panel_top + 130.0f;
        // --- Define a common Y for the top of the text descriptions ---
        float text_top_y = panel_top + 320.0f;

        // --- COLUMN 1: ARROW KEYS ---
        // NEW: Define a center X for this column
        float col1_center_x = panel_left + (panel_w / 4.0f) + 40.0f; // Center of first quarter
        float key_size = 50.0f;
        float key_spacing = 5.0f;

        // Calculate key positions (centered on col1_center_x)
        float up_key_x = col1_center_x;
        float up_key_y = diagram_top_y + (key_size / 2.0f);
        float down_key_x = up_key_x;
        float down_key_y = up_key_y + key_size + key_spacing;
        float left_key_x = up_key_x - key_size - key_spacing;
        float left_key_y = down_key_y;
        float right_key_x = up_key_x + key_size + key_spacing;
        float right_key_y = down_key_y;

        // Draw the key "buttons" with triangles
        DrawKeyWithTriangle(up_key_x, up_key_y, key_size, "UP");
        DrawKeyWithTriangle(left_key_x, left_key_y, key_size, "LEFT");
        DrawKeyWithTriangle(down_key_x, down_key_y, key_size, "DOWN");
        DrawKeyWithTriangle(right_key_x, right_key_y, key_size, "RIGHT");

        // --- COLUMN 2: MOUSE ---
        // NEW: Define a center X for this column
        float col2_center_x = panel_right - (panel_w / 4.0f) - 40.0f; // Center of last quarter
        float mouse_btn_w = 80.0f;
        float mouse_btn_h = 120.0f;
        float mouse_spacing = 10.0f;
        float mouse_font_size = 24.0f;

        // Align the mouse buttons' top to the 'diagram_top_y'
        float btn_y = diagram_top_y + (mouse_btn_h / 2.0f);
        float left_btn_x = col2_center_x - (mouse_btn_w / 2.0f) - (mouse_spacing / 2.0f);
        DrawTutorialButton("MB1", left_btn_x, btn_y, mouse_btn_w, mouse_btn_h, 0, mouse_font_size);
        float right_btn_x = col2_center_x + (mouse_btn_w / 2.0f) + (mouse_spacing / 2.0f);
        DrawTutorialButton("MB2", right_btn_x, btn_y, mouse_btn_w, mouse_btn_h, 0, mouse_font_size);


        // --- ROW 2: TEXT DESCRIPTIONS (CENTERED) ---
        CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
        CP_Settings_TextSize(24);

        // NEW: Set text alignment to CENTER for both blocks
        CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);

        // Draw Key Descriptions (centered on col1_center_x)
        CP_Font_DrawText("UP: Move Up / Navigate", col1_center_x, text_top_y);
        CP_Font_DrawText("LEFT: Move Left / Navigate", col1_center_x, text_top_y + line_height);
        CP_Font_DrawText("DOWN: Move Down / Navigate", col1_center_x, text_top_y + (line_height * 2.0f));
        CP_Font_DrawText("RIGHT: Move Right / Navigate", col1_center_x, text_top_y + (line_height * 3.0f));

        // Draw Mouse Descriptions (centered on col2_center_x)
        CP_Font_DrawText("MB1 (LEFT): Select / Interact", col2_center_x, text_top_y);
        CP_Font_DrawText("MB2 (RIGHT): Cancel / Info", col2_center_x, text_top_y + line_height);


        // --- "Back" Button (Common for page 4) ---
        float back_btn_x = panel_left + 80.0f;
        float back_btn_y = panel_bottom - 45.0f;
        int hover_back = IsAreaClicked(back_btn_x, back_btn_y, TUTE_BUTTON_W, TUTE_BUTTON_H, mouse_x, mouse_y);
        // Reset alignment for the button text (just in case)
        DrawTutorialButton("Back", back_btn_x, back_btn_y, TUTE_BUTTON_W, TUTE_BUTTON_H, hover_back, 24.0f);

        // --- Check for click ---
        if (left_mouse_clicked && hover_back) {
            tutorialPage = 3;
        }
    }


    // --- Draw 'Exit' Text (Common to all pages) ---
    CP_Font_Set(tutorial_font);
    CP_Settings_TextSize(28);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_BOTTOM);
    float exit_text_y = panel_bottom + 40.0f;
    CP_Font_DrawText("Press ESC to return to Main Menu", panel_x, exit_text_y);

    // --- Handle Input (Common to all pages) ---
    if (CP_Input_KeyTriggered(KEY_ESCAPE)) {
        CP_Engine_SetNextGameState(Main_Menu_Init, Main_Menu_Update, Main_Menu_Exit);
    }
}

void Tutorial_Exit(void)
{
    // --- Fixes the memory leak ---
    if (tutorial_font)
    {
        CP_Font_Free(tutorial_font);
    }
}