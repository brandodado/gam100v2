#include "tutorial.h"       
#include "cprocessing.h"    
#include "mainmenu.h"       
#include "utils.h"          
#include <string.h>        

// --- Static variables for this state ---
static CP_Font tutorial_font;
static int tutorialPage;

// --- Button Definitions ---
#define TUTE_BUTTON_W 120.0f
#define TUTE_BUTTON_H 50.0f

// --- MODIFIED: Local enum for card examples ---
typedef enum {
    Attack,
    Heal,
    Shield
} CardType;


// Helper function to draw a single navigation button (e.g., Next/Back).
static void DrawTutorialButton(const char* text, float x, float y, float w, float h, int is_hovered, float fontSize)
{
    CP_Settings_RectMode(CP_POSITION_CENTER);

    // Draw button rectangle
    if (is_hovered) {
        CP_Settings_Fill(CP_Color_Create(150, 150, 150, 255));
    }
    else {
        CP_Settings_Fill(CP_Color_Create(80, 80, 80, 255));
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

// Helper function to draw a keyboard key graphic with a directional triangle icon.
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

// Draws a visual example of a reward card to explain game mechanics.
static void DrawTutorialCardExample(float x, float y, float w, float h, CardType type, const char* title, const char* desc)
{
    CP_Settings_RectMode(CP_POSITION_CENTER);

    // 1. Draw card background color
    switch (type) {
    case Attack:
        CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
        break;
    case Heal:
        CP_Settings_Fill(CP_Color_Create(80, 172, 85, 255));
        break;
    case Shield:
        CP_Settings_Fill(CP_Color_Create(0, 0, 255, 255));
        break;
    }
    CP_Graphics_DrawRect(x, y, w, h);

    // 2. Draw Text
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Font_Set(tutorial_font);
    CP_Settings_TextSize(16);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);

    // Draw Title
    CP_Font_DrawText(title, x, y - (h / 2.0f) + 10.0f);

    // Draw Description (in a text box)
    float padding = 10.0f;
    float text_box_w = w - padding * 2.0f;
    float text_box_x = x - text_box_w / 2.0f;
    float text_box_y = y - (h / 2.0f) + 35.0f;
    CP_Font_DrawTextBox(desc, text_box_x, text_box_y, text_box_w);
}

// Draws a visual example of a buff reward to explain boss mechanics.
static void DrawTutorialBuffExample(float x, float y, float w, float h, const char* title, const char* desc)
{
    CP_Settings_RectMode(CP_POSITION_CENTER);

    // 1. Draw background
    CP_Settings_Fill(CP_Color_Create(50, 50, 80, 255));
    CP_Settings_Stroke(CP_Color_Create(200, 200, 255, 255));
    CP_Settings_StrokeWeight(2.0f);
    CP_Graphics_DrawRect(x, y, w, h);

    // 2. Draw Title Text
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Font_Set(tutorial_font);
    CP_Settings_TextSize(20);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);
    CP_Font_DrawText(title, x, y - (h / 2.0f) + 15.0f);

    // 3. Draw Description Text
    CP_Settings_TextSize(16);
    float text_box_w = w - 30.0f;
    float text_box_x = x - (text_box_w / 2.0f);
    float text_box_y = y - (h / 2.0f) + 50.0f;
    CP_Font_DrawTextBox(desc, text_box_x, text_box_y, text_box_w);
}


// Initializes the tutorial pages and font assets.
void Tutorial_Init(void)
{
    // Load the font for this state
    tutorial_font = CP_Font_Load("Assets/Exo2-Regular.ttf");
    CP_Font_Set(tutorial_font);

    tutorialPage = 1; // Always start on page 1
}

// Renders the current tutorial page and handles navigation between pages.
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
    CP_Settings_TextSize(20);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
    float text_start_x = panel_left + 50.0f;
    float text_start_y = panel_top + 80.0f;
    float text_box_width = panel_w - 100.0f;
    float line_height = 40.0f;

    // --- Multi-page Logic ---

    // --- Page 1 (Controls) ---
    if (tutorialPage == 1)
    {
        // --- Define a common Y for the top of the diagrams ---
        float diagram_top_y = panel_top + 130.0f;
        // --- Define a common Y for the top of the text descriptions ---
        float text_top_y = panel_top + 320.0f;

        // --- COLUMN 1: ARROW KEYS ---
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

        CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);

        // Draw Key Descriptions (centered on col1_center_x)
        // --- MODIFIED: Updated keybinds ---
        CP_Font_DrawText("A / D: Cycle Cards", col1_center_x, text_top_y - line_height);
        CP_Font_DrawText("LEFT/RIGHT: Select Target", col1_center_x, text_top_y);
        CP_Font_DrawText("S: Use Card", col1_center_x, text_top_y + line_height);
        CP_Font_DrawText("ENTER: End Turn", col1_center_x, text_top_y + (line_height * 2.0f));


        // Draw Mouse Descriptions (centered on col2_center_x)
        CP_Font_DrawText("MB1 (LEFT): Select / Interact", col2_center_x, text_top_y);
        CP_Font_DrawText("MB2 (RIGHT): (Not Used)", col2_center_x, text_top_y + line_height);


        // --- No "Back" button on Page 1 ---

        // --- Draw "Next" Button ---
        float next_btn_x = panel_right - 80.0f;
        float next_btn_y = panel_bottom - 45.0f;
        int hover_next = IsAreaClicked(next_btn_x, next_btn_y, TUTE_BUTTON_W, TUTE_BUTTON_H, mouse_x, mouse_y);
        DrawTutorialButton("Next", next_btn_x, next_btn_y, TUTE_BUTTON_W, TUTE_BUTTON_H, hover_next, 24.0f);


        // --- Check for click ---
        if (left_mouse_clicked && hover_next) {
            tutorialPage = 2; // Go to new page 2
        }
    }
    // --- Page 2 ---
    else if (tutorialPage == 2)
    {
        // --- Text for new controls ---
        const char* page1_text = "THE GOAL: Defeat all 9 levels.\n\n"
            "BASIC COMBAT:\n"
            "1. Select a card with (A/D) or (Mouse Click). First card is auto-selected.\n"
            "2. Press (S) to use the selected card (Heal/Shield on you, Attack on enemy).\n"
            "3. (Alternative) Click the player to use Heal/Shield or click an enemy to use Attack.\n"
            "4. Press (ENTER) or click 'End Turn' when you are finished.\n"
            "5. If you play your last card, your turn will end automatically.";

        CP_Font_DrawTextBox(page1_text, text_start_x, text_start_y, text_box_width);

        CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
        CP_Settings_TextSize(18);
        CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
        CP_Font_DrawText("Look for this button in combat:", panel_x, panel_y + 40.0f);

        DrawTutorialButton("Use Card (S)", panel_x - 100.0f, panel_y + 100.0f, 150.0f, 70.0f, 0, 18.0f);
        DrawTutorialButton("End Turn (Enter)", panel_x + 100.0f, panel_y + 100.0f, 150.0f, 70.0f, 0, 18.0f);


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
    // --- Page 3 ---
    else if (tutorialPage == 3)
    {
        const char* page2_text = "HOW TO GET STRONGER (1/2): CARD REWARDS\n\n"
            "After normal levels (1, 2, 4, etc.), you will be offered 3 special AOE cards.\n"
            "Choosing one adds 2 copies to your deck (the first time) AND permanently upgrades all cards of that type!";

        CP_Font_DrawTextBox(page2_text, text_start_x, text_start_y, text_box_width);

        float card_w = 120.0f;
        float card_h = 180.0f;
        float card_y = panel_y + 60.0f;
        float card_spacing = 150.0f;

        DrawTutorialCardExample(panel_x - card_spacing, card_y, card_w, card_h, Attack,
            "Cleave", "Deals X damage to ALL enemies.\n\nall Attack cards do an additional +Y damage.");

        DrawTutorialCardExample(panel_x, card_y, card_w, card_h, Heal,
            "Divine Strike", "Heals X HP. Deals 50% of heal as AOE damage.\n\nall Heal cards do an additional +Y HP.");

        DrawTutorialCardExample(panel_x + card_spacing, card_y, card_w, card_h, Shield,
            "Shield Bash", "Gain X Shield, then deal damage equal to 75% of your new Shield to ALL enemies.");


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
                tutorialPage = 2;
            }
            else if (hover_next) {
                tutorialPage = 4;
            }
        }
    }
    // --- Page 4 ---
    else if (tutorialPage == 4)
    {
        const char* page3_text = "HOW TO GET STRONGER (2/2): BUFF REWARDS\n\n"
            "After defeating a boss (levels 3, 6, 9), you will choose 1 of 3 powerful, permanent passive buffs.";

        CP_Font_DrawTextBox(page3_text, text_start_x, text_start_y, text_box_width);

        float buff_w = 250.0f;
        float buff_h = 140.0f;
        float buff_x = panel_x;
        float buff_y_top = panel_y;
        float buff_y_bottom = panel_y + 160.0f;

        DrawTutorialBuffExample(buff_x, buff_y_top, buff_w, buff_h,
            "Reinforce (Lvl 3)", "Permanently increase all Shield gains by 25%.");

        DrawTutorialBuffExample(buff_x, buff_y_bottom, buff_w, buff_h,
            "Power Infusion (Lvl 6)", "All Attack cards are permanently 35% stronger.");

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
                tutorialPage = 3;
            }
            else if (hover_next) {
                tutorialPage = 5;
            }
        }
    }
    // --- Page 5 ---
    else if (tutorialPage == 5)
    {
        const char* page4_text = "BOSSES & CHECKPOINTS\n\n"
            "BOSSES: Bosses 'Enrage' at the start of your turn, gaining permanent Attack. Defeat them quickly!\n\n"
            "CHECKPOINTS: If you die, you can click 'Restart Stage' on the Game Over screen to retry the level.";

        CP_Font_DrawTextBox(page4_text, text_start_x, text_start_y, text_box_width);

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
                tutorialPage = 4;
            }
            else if (hover_next) {
                tutorialPage = 6;
            }
        }
    }
    // --- Page 6 ---
    else if (tutorialPage == 6)
    {
        const char* page6_text = "VICTORY & SCORING\n\n"
            "After defeating the final boss on Level 9, you will reach the Victory Screen.\n\n"
            "You will be awarded a star rating based on your performance (tracked by the 'Restarts' counter):\n\n"
            "   *** (3 Stars) - 0 Restarts (Flawless)\n"
            "   ** (2 Stars) - 1-2 Restarts\n"
            "   * (1 Star)  - 3 or more Restarts\n\n"
            "Try to achieve a 3-star victory!";

        CP_Font_DrawTextBox(page6_text, text_start_x, text_start_y, text_box_width);

        // --- "Back" Button ---
        float back_btn_x = panel_left + 80.0f;
        float back_btn_y = panel_bottom - 45.0f;
        int hover_back = IsAreaClicked(back_btn_x, back_btn_y, TUTE_BUTTON_W, TUTE_BUTTON_H, mouse_x, mouse_y);
        DrawTutorialButton("Back", back_btn_x, back_btn_y, TUTE_BUTTON_W, TUTE_BUTTON_H, hover_back, 24.0f);

        // --- Check for click ---
        if (left_mouse_clicked && hover_back) {
            tutorialPage = 5; // Go back to page 5
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

// Frees the font resource for the tutorial state.
void Tutorial_Exit(void)
{
    // --- Fixes the memory leak ---
    if (tutorial_font)
    {
        CP_Font_Free(tutorial_font);
    }
}