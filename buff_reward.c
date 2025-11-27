#define _CRT_SECURE_NO_WARNINGS // <-- ADDED: Suppresses strncpy warnings
#include "buff_reward.h"
#include "game.h"       // Include game.h to get the full Player struct definition
#include "utils.h"      // For IsAreaClicked
#include <stdio.h>      // For snprintf

// --- Static variables for this state ---
static CP_Font buff_font;
static bool font_loaded = false;

#define BUFF_OPTION_W 250.0f
#define BUFF_OPTION_H 150.0f
#define BUFF_SPACING 50.0f

// --- Helper function to apply the chosen buff ---
static void ApplyBuffReward(BuffRewardState* state, Player* player) {
    if (!player || state->selected_index < 0 || state->selected_index >= state->num_options) {
        return;
    }

    BuffType selected_buff = state->options[state->selected_index].type;

    // --- MODIFIED: Calculate scaling buff amount ---
    int boss_count = 0;
    if (player->max_health > 50) boss_count++; // Simple check if Lvl 3 buff was taken
    if (player->max_health > 70) boss_count++; // Simple check if Lvl 6 buff was taken

    // int hp_gain = 20 + (10 * boss_count); // <-- No longer used
    int attack_gain = 2 + (1 * boss_count);


    switch (selected_buff) {
    case BUFF_LIFESTEAL:
        player->has_lifesteal = true;
        break;
    case BUFF_DESPERATE_DRAW:
        player->has_desperate_draw = true;
        break;
    case BUFF_DIVINE_STRIKE:
        player->has_divine_strike = true;
        break;
    case BUFF_SHIELD_BOOST:
        player->has_shield_boost = true;
        break;
    case BUFF_ATTACK_UP:
        // --- MODIFIED: Use scaling value ---
        player->attack += attack_gain;
        break;
        // --- MODIFIED: Added Lvl 6 Buffs ---
    case BUFF_ATTACK_BOOST_35:
        player->has_attack_boost_35 = true;
        break;
    case BUFF_HEAL_BOOST_35:
        player->has_heal_boost_35 = true;
        break;
    case BUFF_SHIELD_BOOST_35:
        player->has_shield_boost_35 = true;
        break;
    case BUFF_NONE:
        // Do nothing
        break;
    }
}


void InitBuffReward(BuffRewardState* state) {
    if (!state) return;

    state->num_options = 0;
    state->is_active = false;
    state->reward_claimed = false;
    state->selected_index = -1;
    state->option_w = BUFF_OPTION_W;
    state->option_h = BUFF_OPTION_H;
    state->show_confirm_button = false;

    if (!font_loaded) {
        buff_font = CP_Font_Load("Assets/Roboto-Regular.ttf");
        font_loaded = true;
    }

    // Pre-calculate positions
    float ww = (float)CP_System_GetWindowWidth();
    float wh = (float)CP_System_GetWindowHeight();
    float total_width = (MAX_BUFF_OPTIONS * BUFF_OPTION_W) + ((MAX_BUFF_OPTIONS - 1) * BUFF_SPACING);
    float start_x = (ww - total_width) / 2.0f;
    float y_pos = wh / 2.0f;

    for (int i = 0; i < MAX_BUFF_OPTIONS; i++) {
        state->option_pos[i] = CP_Vector_Set(
            start_x + i * (BUFF_OPTION_W + BUFF_SPACING) + (BUFF_OPTION_W / 2.0f),
            y_pos
        );
    }
}

// --- MODIFIED: Added current_level to generate dynamic buff descriptions ---
void GenerateBuffOptions(BuffRewardState* state, int current_level) {
    if (!state) return;

    // --- MODIFIED: Calculate scaling buff amounts ---
    int boss_count = (current_level / 3) - 1; // Lvl 3 -> 0, Lvl 6 -> 1, Lvl 9 -> 2
    int attack_gain = 2 + (1 * boss_count);


    // --- MODIFIED: Different buffs for Lvl 3, 6, and 9 ---
    if (current_level == 3) {
        // --- Lvl 3 Buffs ---
        state->options[0].type = BUFF_LIFESTEAL;
        state->options[0].title = "Vampiric Strike";
        snprintf(state->options[0].description, sizeof(state->options[0].description), "Heal for 50%% of damage you deal with Attack cards.");

        state->options[1].type = BUFF_DESPERATE_DRAW;
        state->options[1].title = "Card Mastery";
        snprintf(state->options[1].description, sizeof(state->options[1].description), "Draw 1 additional card at the start of each turn.");

        state->options[2].type = BUFF_SHIELD_BOOST;
        state->options[2].title = "Reinforce";
        snprintf(state->options[2].description, sizeof(state->options[2].description), "Permanently increase all Shield gains by 25%%.");
    }
    else if (current_level == 6) {
        // --- Lvl 6 Buffs ---
        state->options[0].type = BUFF_ATTACK_BOOST_35;
        state->options[0].title = "Power Infusion";
        snprintf(state->options[0].description, sizeof(state->options[0].description), "All Attack cards are permanently 35%% stronger.");

        state->options[1].type = BUFF_HEAL_BOOST_35;
        state->options[1].title = "Holy Infusion";
        snprintf(state->options[1].description, sizeof(state->options[1].description), "All Heal cards are permanently 35%% stronger.");

        state->options[2].type = BUFF_SHIELD_BOOST_35;
        state->options[2].title = "Barrier Infusion";
        snprintf(state->options[2].description, sizeof(state->options[2].description), "All Shield cards are permanently 35%% stronger.");
    }
    else {
        // --- Lvl 9 Buffs ---
        state->options[0].type = BUFF_LIFESTEAL;
        state->options[0].title = "Vampiric Strike";
        snprintf(state->options[0].description, sizeof(state->options[0].description), "Heal for 50%% of damage you deal with Attack cards.");

        state->options[1].type = BUFF_DESPERATE_DRAW;
        state->options[1].title = "Card Mastery";
        snprintf(state->options[1].description, sizeof(state->options[1].description), "Draw 1 additional card at the start of each turn.");

        state->options[2].type = BUFF_ATTACK_UP;
        state->options[2].title = "Rage";
        snprintf(state->options[2].description, sizeof(state->options[2].description), "Permanently increase your base Attack by %d.", attack_gain);
    }

    state->num_options = 3;
    state->is_active = true;
    state->reward_claimed = false;
    state->selected_index = -1;
    state->show_confirm_button = false;
}

void UpdateBuffReward(BuffRewardState* state, Player* player) {
    if (!state || !state->is_active || state->reward_claimed) return;

    if (CP_Input_MouseClicked()) {
        float mouse_x = (float)CP_Input_GetMouseX();
        float mouse_y = (float)CP_Input_GetMouseY();

        // Check confirm button first
        if (state->show_confirm_button) {
            float ww = (float)CP_System_GetWindowWidth();
            float wh = (float)CP_System_GetWindowHeight();
            float btn_x = ww / 2.0f;
            float btn_y = wh - 150.0f;
            float btn_w = 250.0f;
            float btn_h = 60.0f;

            if (IsAreaClicked(btn_x, btn_y, btn_w, btn_h, mouse_x, mouse_y)) {
                // Confirmed - apply buff
                ApplyBuffReward(state, player);
                state->reward_claimed = true;
                state->is_active = false;
                return;
            }
        }

        // Check buff options
        for (int i = 0; i < state->num_options; i++) {
            if (IsAreaClicked(state->option_pos[i].x, state->option_pos[i].y, state->option_w, state->option_h, mouse_x, mouse_y)) {
                state->selected_index = i;
                state->show_confirm_button = true; // Show confirm button
                return;
            }
        }
    }
}

void DrawBuffReward(BuffRewardState* state) {
    if (!state || !state->is_active) return;

    float ww = (float)CP_System_GetWindowWidth();
    float wh = (float)CP_System_GetWindowHeight();

    // 1. Draw Background Overlay
    CP_Settings_RectMode(CP_POSITION_CORNER);
    CP_Settings_Fill(CP_Color_Create(0, 0, 0, 200));
    CP_Graphics_DrawRect(0, 0, ww, wh);

    // 2. Draw Title
    CP_Font_Set(buff_font);
    CP_Settings_TextSize(40);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Settings_Fill(CP_Color_Create(255, 255, 0, 255));
    CP_Font_DrawText("Choose Your Power-Up!", ww / 2.0f, 150.0f);

    // 3. Draw Buff Options
    CP_Settings_RectMode(CP_POSITION_CENTER);
    float mouse_x = (float)CP_Input_GetMouseX();
    float mouse_y = (float)CP_Input_GetMouseY();

    for (int i = 0; i < state->num_options; i++) {
        CP_Vector pos = state->option_pos[i];

        bool is_hovered = IsAreaClicked(pos.x, pos.y, state->option_w, state->option_h, mouse_x, mouse_y);

        // Draw border/glow
        if (is_hovered) {
            CP_Settings_Fill(CP_Color_Create(255, 255, 0, 150));
            CP_Settings_NoStroke();
            CP_Graphics_DrawRect(pos.x, pos.y, state->option_w + 10.0f, state->option_h + 10.0f);
        }

        // Draw background
        CP_Settings_Fill(CP_Color_Create(50, 50, 80, 255));
        CP_Settings_Stroke(CP_Color_Create(200, 200, 255, 255));
        CP_Settings_StrokeWeight(2.0f);
        CP_Graphics_DrawRect(pos.x, pos.y, state->option_w, state->option_h);

        // Draw Title Text
        CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
        CP_Settings_TextSize(24);
        CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);
        CP_Font_DrawText(state->options[i].title, pos.x, pos.y - (state->option_h / 2.0f) + 15.0f);

        // Draw Description Text
        CP_Settings_TextSize(16);
        CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);
        float text_box_w = state->option_w - 30.0f;
        float text_box_x = pos.x - (text_box_w / 2.0f);
        float text_box_y = pos.y - (state->option_h / 2.0f) + 50.0f;
        CP_Font_DrawTextBox(state->options[i].description, text_box_x, text_box_y, text_box_w);
    }
    // 4. Draw Confirm Button
    if (state->show_confirm_button && !state->reward_claimed) {
        float btn_x = ww / 2.0f;
        float btn_y = wh - 150.0f;
        float btn_w = 250.0f;
        float btn_h = 60.0f;

        bool is_hovered = IsAreaClicked(btn_x, btn_y, btn_w, btn_h, mouse_x, mouse_y);

        // Button background
        CP_Settings_Fill(is_hovered ? CP_Color_Create(80, 200, 80, 255) : CP_Color_Create(50, 150, 50, 255));
        CP_Settings_Stroke(CP_Color_Create(255, 255, 255, 255));
        CP_Settings_StrokeWeight(3);
        CP_Settings_RectMode(CP_POSITION_CENTER);
        CP_Graphics_DrawRect(btn_x, btn_y, btn_w, btn_h);

        // Button text
        CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
        CP_Settings_TextSize(28);
        CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
        CP_Font_DrawText("CONFIRM", btn_x, btn_y);
    }

    // 5. Footer text
    CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255));
    CP_Settings_TextSize(20);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);

    if (state->show_confirm_button) {
        CP_Font_DrawText("Click CONFIRM to gain this power-up", ww / 2.0f, wh - 80.0f);
    }
    else {
        CP_Font_DrawText("Click a power-up to select", ww / 2.0f, wh - 80.0f);
    }
}

void ResetBuffReward(BuffRewardState* state) {
    if (!state) return;
    state->num_options = 0;
    state->is_active = false;
    state->reward_claimed = false;
    state->selected_index = -1;
}

bool IsBuffRewardActive(BuffRewardState* state) {
    if (!state) return false;
    return state->is_active;
}