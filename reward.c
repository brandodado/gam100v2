#include "reward.h"
#include "cprocessing.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define REWARD_CARD_SPACING 150
#define REWARD_CARD_Y 300
#define SKIP_BUTTON_W 200
#define SKIP_BUTTON_H 60

static CP_Font reward_font;
static bool font_loaded = false;

void InitReward(RewardState* reward_state) {
    if (!reward_state) return;

    reward_state->num_options = 0;
    reward_state->is_active = false;
    reward_state->reward_claimed = false;
    reward_state->selected_index = -1;

    // Load font 
    if (!font_loaded) {
        reward_font = CP_Font_Load("Assets/Roboto.ttf");
        font_loaded = true;
    }
}

void GenerateRewardOptions(RewardState* reward_state) {
    if (!reward_state) return;

    CP_Vector card_pos = CP_Vector_Set(0, 0);

    // Create attack card reward
    Card attack_reward = {
        card_pos,
        card_pos,
        Attack,
        None,
        5,
        "Deal 5 attack damage",
        CARD_W_INIT * 1.5f,  // Make reward cards slightly bigger
        CARD_H_INIT * 1.5f,
        false
    };

    // Create heal card reward
    Card heal_reward = {
        card_pos,
        card_pos,
        Heal,
        None,
        7,
        "Heal 7 health points",
        CARD_W_INIT * 1.5f,
        CARD_H_INIT * 1.5f,
        false
    };

    // Add both options
    reward_state->options[0].card = attack_reward;
    reward_state->options[0].type = REWARD_ATTACK_CARD;
    reward_state->options[0].is_selected = false;

    reward_state->options[1].card = heal_reward;
    reward_state->options[1].type = REWARD_HEAL_CARD;
    reward_state->options[1].is_selected = false;

    reward_state->num_options = 2;
    reward_state->is_active = true;
    reward_state->reward_claimed = false;
    reward_state->selected_index = -1;
}

bool IsRewardOptionClicked(RewardState* reward_state, int index, float mouse_x, float mouse_y) {
    if (!reward_state || index < 0 || index >= reward_state->num_options) {
        return false;
    }

    float window_width = (float)CP_System_GetWindowWidth();
    float total_width = reward_state->num_options * CARD_W_INIT * 1.5f +
        (reward_state->num_options - 1) * REWARD_CARD_SPACING;
    float start_x = (window_width - total_width) / 2.0f + (CARD_W_INIT * 1.5f) / 2.0f;

    float card_x = start_x + index * (CARD_W_INIT * 1.5f + REWARD_CARD_SPACING);
    float card_y = REWARD_CARD_Y;

    return IsAreaClicked(card_x, card_y,
        reward_state->options[index].card.card_w,
        reward_state->options[index].card.card_h,
        mouse_x, mouse_y);
}

void UpdateReward(RewardState* reward_state, Deck* deck) {
    if (!reward_state || !reward_state->is_active || reward_state->reward_claimed) return;

    // Check for mouse clicks on reward options
    if (CP_Input_MouseClicked()) {
        float mouse_x = CP_Input_GetMouseX();
        float mouse_y = CP_Input_GetMouseY();

        // Check skip button
        float window_width = (float)CP_System_GetWindowWidth();
        float window_height = (float)CP_System_GetWindowHeight();
        float skip_btn_x = window_width / 2;
        float skip_btn_y = window_height - 150;

        if (IsAreaClicked(skip_btn_x, skip_btn_y, SKIP_BUTTON_W, SKIP_BUTTON_H, mouse_x, mouse_y)) {
            SkipReward(reward_state);
            return;
        }

        // Check card options
        for (int i = 0; i < reward_state->num_options; i++) {
            if (IsRewardOptionClicked(reward_state, i, mouse_x, mouse_y)) {
                reward_state->selected_index = i;

                // Apply reward immediately and lock selection
                if (ApplyRewardSelection(reward_state, deck)) {
                    reward_state->reward_claimed = true;
                    reward_state->is_active = false;
                }
                break;
            }
        }
    }
}

void SkipReward(RewardState* reward_state) {
    if (!reward_state) return;

    reward_state->reward_claimed = true;
    reward_state->is_active = false;
    reward_state->selected_index = -1; // No card selected
}

void DrawReward(RewardState* reward_state) {
    if (!reward_state || (!reward_state->is_active && !reward_state->reward_claimed)) return;

    float window_width = (float)CP_System_GetWindowWidth();
    float window_height = (float)CP_System_GetWindowHeight();

    // 1. Draw Background Overlay (Using CORNER mode)
    CP_Settings_RectMode(CP_POSITION_CORNER);
    CP_Settings_Fill(CP_Color_Create(0, 0, 0, 200));
    CP_Graphics_DrawRect(0, 0, window_width, window_height);

    // 2. Draw Title
    CP_Font_Set(reward_font);
    CP_Settings_TextSize(40);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Settings_Fill(CP_Color_Create(255, 255, 0, 255));
    CP_Font_DrawText("Choose Your Reward!", window_width / 2, 100);

    // --- CRITICAL FIX START ---
    // Switch to CENTER mode because your card math calculates the center point.
    CP_Settings_RectMode(CP_POSITION_CENTER);
    // --- CRITICAL FIX END ---

    float total_width = reward_state->num_options * CARD_W_INIT * 1.5f +
        (reward_state->num_options - 1) * REWARD_CARD_SPACING;
    float start_x = (window_width - total_width) / 2.0f + (CARD_W_INIT * 1.5f) / 2.0f;

    // 3. Draw Cards Loop
    for (int i = 0; i < reward_state->num_options; i++) {
        float card_x = start_x + i * (CARD_W_INIT * 1.5f + REWARD_CARD_SPACING);
        float card_y = REWARD_CARD_Y;

        // Update position for logic
        reward_state->options[i].card.pos = CP_Vector_Set(card_x, card_y);

        // A. HANDLE REWARD CLAIMED STATE
        if (reward_state->reward_claimed) {
            if (i == reward_state->selected_index) {
                // Draw a Green Box BEHIND the selected card
                CP_Settings_Fill(CP_Color_Create(0, 255, 0, 100));
                CP_Settings_NoStroke();
                CP_Graphics_DrawRect(card_x, card_y,
                    reward_state->options[i].card.card_w * 1.1f,
                    reward_state->options[i].card.card_h * 1.1f);
            }
            else {
                // Draw Gray overlay ON TOP of non-selected cards
                DrawCard(&reward_state->options[i].card); // Draw card first
                CP_Settings_Fill(CP_Color_Create(0, 0, 0, 150));
                CP_Graphics_DrawRect(card_x, card_y,
                    reward_state->options[i].card.card_w,
                    reward_state->options[i].card.card_h);
                continue; // Skip the text labels for non-selected cards
            }
        }
        // B. HANDLE NORMAL SELECTION STATE
        else {
            float mouse_x = CP_Input_GetMouseX();
            float mouse_y = CP_Input_GetMouseY();

            if (IsRewardOptionClicked(reward_state, i, mouse_x, mouse_y)) {
                // Draw Yellow Glow BEHIND card
                CP_Settings_Fill(CP_Color_Create(255, 255, 0, 150)); // Semi-transparent yellow
                CP_Settings_NoStroke();
                CP_Graphics_DrawRect(card_x, card_y,
                    reward_state->options[i].card.card_w * 1.1f,
                    reward_state->options[i].card.card_h * 1.1f);
            }
        }

        // Reset settings before drawing the card content to prevent color bleed
        CP_Settings_Stroke(CP_Color_Create(0, 0, 0, 255));
        CP_Settings_StrokeWeight(1);

        DrawCard(&reward_state->options[i].card);

        // Draw Labels
        CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
        CP_Settings_TextSize(24);

        const char* label = (reward_state->options[i].type == REWARD_ATTACK_CARD) ? "Attack Card" : "Heal Card";

        float text_y = card_y + reward_state->options[i].card.card_h / 2 + 40;
        CP_Font_DrawText(label, card_x, text_y); // Centered X (since RectMode is Center)

        CP_Settings_TextSize(16);
        CP_Font_DrawText(reward_state->options[i].card.description, card_x, text_y + 30);
    }

    // 4. Draw Skip Button
    float skip_btn_x = window_width / 2;
    float skip_btn_y = window_height - 150;
    float mx = CP_Input_GetMouseX();
    float my = CP_Input_GetMouseY();

    if (IsAreaClicked(skip_btn_x, skip_btn_y, SKIP_BUTTON_W, SKIP_BUTTON_H, mx, my)) {
        CP_Settings_Fill(CP_Color_Create(150, 50, 50, 255));
    }
    else {
        CP_Settings_Fill(CP_Color_Create(100, 100, 100, 255));
    }

    // Ensure we are still in Center Mode (We set it earlier, but good for safety)
    CP_Settings_RectMode(CP_POSITION_CENTER);
    CP_Graphics_DrawRect(skip_btn_x, skip_btn_y, SKIP_BUTTON_W, SKIP_BUTTON_H);

    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Settings_TextSize(24);
    CP_Font_DrawText("Skip Reward", skip_btn_x, skip_btn_y); // Centered text

    // 5. Draw Footer Text
    CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255));
    CP_Settings_TextSize(20);

    if (reward_state->reward_claimed) {
        CP_Font_DrawText("Proceeding to next level...", window_width / 2, window_height - 100);
    }
    else {
        CP_Font_DrawText("Click a card to add to deck", window_width / 2, window_height - 100);
    }
}

bool ApplyRewardSelection(RewardState* reward_state, Deck* deck) {
    if (!reward_state || !deck || reward_state->selected_index < 0 ||
        reward_state->selected_index >= reward_state->num_options) {
        return false;
    }

    // Get the selected card and reset its position for deck
    Card selected_card = reward_state->options[reward_state->selected_index].card;
    selected_card.pos = CP_Vector_Set(50, 600);
    selected_card.target_pos = CP_Vector_Set(0, 0);
    selected_card.card_w = CARD_W_INIT;
    selected_card.card_h = CARD_H_INIT;
    selected_card.is_animating = false;

    // Add to deck
    return AddCardToDeck(deck, selected_card);
}

void ResetReward(RewardState* reward_state) {
    if (!reward_state) return;

    reward_state->num_options = 0;
    reward_state->is_active = false;
    reward_state->reward_claimed = false;
    reward_state->selected_index = -1;
}

bool IsRewardActive(RewardState* reward_state) {
    if (!reward_state) return false;
    return reward_state->is_active;
}