#define _CRT_SECURE_NO_WARNINGS 
#include "reward.h"
#include "cprocessing.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Matching scale factor
#define CARD_SCALE 1.5f

#define REWARD_CARD_SPACING 150
#define REWARD_CARD_Y 300

static CP_Font reward_font;
static bool font_loaded = false;

void InitReward(RewardState* reward_state) {
    if (!reward_state) return;

    reward_state->num_options = 0;
    reward_state->is_active = false;
    reward_state->reward_claimed = false;
    reward_state->selected_index = -1;
    reward_state->show_confirm_button = false;

    if (!font_loaded) {
        reward_font = CP_Font_Load("Assets/Roboto-Regular.ttf");
        font_loaded = true;
    }
}

// This function creates 3 choices for the player to pick from
// It dynamically generates the descriptions based on current stats
void GenerateRewardOptions(RewardState* reward_state, Player* player) {
    if (!reward_state || !player) return;

    CP_Vector card_pos = CP_Vector_Set(0, 0);

    float card_w = CARD_W_INIT * 4;
    float card_h = CARD_H_INIT * 4;

    // Calculate how much the bonus increases this time
    int buff_increase = 2 + player->card_reward_count;

    // Calculate future stats so we can show them to the player
    int next_attack_bonus = player->attack_bonus + buff_increase;
    int next_heal_bonus = player->heal_bonus + buff_increase;
    int next_shield_bonus = player->shield_bonus + buff_increase;

    char attack_desc[200];
    char heal_desc[200];
    char shield_desc[200];

    // Conditional descriptions: If it's the first time, we mention adding special cards.
    // Otherwise, we just mention the passive buff increase.
    if (player->attack_bonus == 0) {
        snprintf(attack_desc, sizeof(attack_desc), "Add 2 Cleave.\n\nAll Atk Cards\n+%d Dmg.", next_attack_bonus);
    }
    else {
        snprintf(attack_desc, sizeof(attack_desc), "All Atk Cards\n+%d Dmg.", next_attack_bonus);
    }

    if (player->heal_bonus == 0) {
        snprintf(heal_desc, sizeof(heal_desc), "Add 2 Divine.\n\nAll Heal Cards\n+%d HP.", next_heal_bonus);
    }
    else {
        snprintf(heal_desc, sizeof(heal_desc), "All Heal Cards\n+%d HP.", next_heal_bonus);
    }

    if (player->shield_bonus == 0) {
        snprintf(shield_desc, sizeof(shield_desc), "Add 2 Bash.\n\nAll Shield Cards\n+%d Shield.", next_shield_bonus);
    }
    else {
        snprintf(shield_desc, sizeof(shield_desc), "All Shield Cards\n+%d Shield.", next_shield_bonus);
    }


    // Setup the actual Card data structures for the UI to draw
    Card attack_reward = {
        card_pos, card_pos, Attack, CLEAVE, 7, "",
        card_w, card_h, false, false
    };
    strncpy(attack_reward.description, attack_desc, sizeof(attack_reward.description) - 1);


    Card heal_reward = {
        card_pos, card_pos, Heal, DIVINE_STRIKE_EFFECT, 7, "",
        card_w, card_h, false, false
    };
    strncpy(heal_reward.description, heal_desc, sizeof(heal_reward.description) - 1);

    Card shield_reward = {
        card_pos, card_pos, Shield, SHIELD_BASH, 5, "",
        card_w, card_h, false, false
    };
    strncpy(shield_reward.description, shield_desc, sizeof(shield_reward.description) - 1);


    // Store them in the reward state
    reward_state->options[0].card = attack_reward;
    reward_state->options[0].type = REWARD_ATTACK_CARD;
    reward_state->options[0].is_selected = false;

    reward_state->options[1].card = heal_reward;
    reward_state->options[1].type = REWARD_HEAL_CARD;
    reward_state->options[1].is_selected = false;

    reward_state->options[2].card = shield_reward;
    reward_state->options[2].type = REWARD_SHIELD_CARD;
    reward_state->options[2].is_selected = false;

    reward_state->num_options = 3;
    reward_state->is_active = true;
    reward_state->reward_claimed = false;
    reward_state->selected_index = -1;
    reward_state->show_confirm_button = false;
}

// Checks if mouse clicked on a specific card option
bool IsRewardOptionClicked(RewardState* reward_state, int index, float mouse_x, float mouse_y) {
    if (!reward_state || index < 0 || index >= reward_state->num_options) {
        return false;
    }

    float window_width = (float)CP_System_GetWindowWidth();

    float card_w = CARD_W_INIT * CARD_SCALE;

    // Center the group of cards horizontally
    float total_width = reward_state->num_options * card_w +
        (reward_state->num_options - 1) * REWARD_CARD_SPACING;
    float start_x = (window_width - total_width) / 2.0f + (card_w / 2.0f);

    float card_x = start_x + index * (card_w + REWARD_CARD_SPACING);
    float card_y = REWARD_CARD_Y;

    return IsAreaClicked(card_x, card_y,
        reward_state->options[index].card.card_w,
        reward_state->options[index].card.card_h,
        mouse_x, mouse_y);
}

void UpdateReward(RewardState* reward_state, Deck* deck, Player* player) {
    if (!reward_state || !reward_state->is_active || reward_state->reward_claimed) return;

    if (CP_Input_MouseClicked()) {
        float mouse_x = (float)CP_Input_GetMouseX();
        float mouse_y = (float)CP_Input_GetMouseY();

        // Check confirm button first
        if (reward_state->show_confirm_button) {
            float window_width = (float)CP_System_GetWindowWidth();
            float window_height = (float)CP_System_GetWindowHeight();
            float btn_x = window_width / 2.0f;
            float btn_y = window_height - 150.0f;
            float btn_w = 250.0f;
            float btn_h = 60.0f;

            if (IsAreaClicked(btn_x, btn_y, btn_w, btn_h, mouse_x, mouse_y)) {
                // Confirmed - apply reward
                ApplyRewardSelection(reward_state, deck, player);
                reward_state->reward_claimed = true;
                reward_state->is_active = false;
                return;
            }
        }

        // Check card options
        for (int i = 0; i < reward_state->num_options; i++) {
            if (IsRewardOptionClicked(reward_state, i, mouse_x, mouse_y)) {
                reward_state->selected_index = i;
                reward_state->show_confirm_button = true; // Show confirm button
                break;
            }
        }
    }
}

void DrawReward(RewardState* reward_state, Player* player) {
    if (!reward_state || !player || (!reward_state->is_active && !reward_state->reward_claimed)) return;

    float window_width = (float)CP_System_GetWindowWidth();
    float window_height = (float)CP_System_GetWindowHeight();

    // 1. Darken background
    CP_Settings_RectMode(CP_POSITION_CORNER);
    CP_Settings_Fill(CP_Color_Create(0, 0, 0, 200));
    CP_Graphics_DrawRect(0, 0, window_width, window_height);

    // 2. Draw Title
    CP_Font_Set(reward_font);
    CP_Settings_TextSize(40);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Settings_Fill(CP_Color_Create(255, 255, 0, 255));
    CP_Font_DrawText("Choose Your Reward!", window_width / 2.0f, 100.0f);

    CP_Settings_RectMode(CP_POSITION_CENTER);

    float card_w = CARD_W_INIT * CARD_SCALE;

    float total_width = reward_state->num_options * card_w +
        (reward_state->num_options - 1) * REWARD_CARD_SPACING;
    float start_x = (window_width - total_width) / 2.0f + (card_w / 2.0f);

    // 3. Draw Cards Loop
    for (int i = 0; i < reward_state->num_options; i++) {
        float card_x = start_x + i * (card_w + REWARD_CARD_SPACING);
        float card_y = REWARD_CARD_Y;

        reward_state->options[i].card.pos = CP_Vector_Set(card_x, card_y);

        // Draw highlights logic
        if (reward_state->reward_claimed) {
            if (i == reward_state->selected_index) {
                // Green highlight for selected
                CP_Settings_Fill(CP_Color_Create(0, 255, 0, 100));
                CP_Settings_NoStroke();
                CP_Graphics_DrawRect(card_x, card_y,
                    reward_state->options[i].card.card_w * 1.1f,
                    reward_state->options[i].card.card_h * 1.1f);
            }
            else {
                // Grey out others
                DrawCard(&reward_state->options[i].card, player);
                CP_Settings_Fill(CP_Color_Create(0, 0, 0, 150));
                CP_Graphics_DrawRect(card_x, card_y,
                    reward_state->options[i].card.card_w,
                    reward_state->options[i].card.card_h);
                continue;
            }
        }
        else {
            // Hover effect
            float mouse_x = (float)CP_Input_GetMouseX();
            float mouse_y = (float)CP_Input_GetMouseY();

            if (IsRewardOptionClicked(reward_state, i, mouse_x, mouse_y)) {
                CP_Settings_Fill(CP_Color_Create(255, 255, 0, 150));
                CP_Settings_NoStroke();
                CP_Graphics_DrawRect(card_x, card_y,
                    reward_state->options[i].card.card_w * 1.1f,
                    reward_state->options[i].card.card_h * 1.1f);
            }
        }

        // Reset for drawing card content
        CP_Settings_Stroke(CP_Color_Create(0, 0, 0, 255));
        CP_Settings_StrokeWeight(1);

        DrawCard(&reward_state->options[i].card, player);

        // Draw "Current Bonus" text below card
        CP_Settings_Fill(CP_Color_Create(220, 220, 255, 255));
        CP_Settings_TextSize(18);
        CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);

        char current_bonus_text[100];
        if (reward_state->options[i].type == REWARD_ATTACK_CARD) {
            snprintf(current_bonus_text, sizeof(current_bonus_text), "Current Bonus: +%d", player->attack_bonus);
        }
        else if (reward_state->options[i].type == REWARD_HEAL_CARD) {
            snprintf(current_bonus_text, sizeof(current_bonus_text), "Current Bonus: +%d", player->heal_bonus);
        }
        else { // Shield
            snprintf(current_bonus_text, sizeof(current_bonus_text), "Current Bonus: +%d", player->shield_bonus);
        }

        float text_y = card_y + reward_state->options[i].card.card_h / 2 + 30;
        CP_Font_DrawText(current_bonus_text, card_x, text_y);
    }

    // 4. Draw Confirm Button
    if (reward_state->show_confirm_button && !reward_state->reward_claimed) {
        float window_width = (float)CP_System_GetWindowWidth();
        float window_height = (float)CP_System_GetWindowHeight();
        float btn_x = window_width / 2.0f;
        float btn_y = window_height - 150.0f;
        float btn_w = 250.0f;
        float btn_h = 60.0f;

        float mouse_x = (float)CP_Input_GetMouseX();
        float mouse_y = (float)CP_Input_GetMouseY();
        bool is_hovered = IsAreaClicked(btn_x, btn_y, btn_w, btn_h, mouse_x, mouse_y);

        // Button background
        CP_Settings_Fill(is_hovered ? CP_Color_Create(80, 200, 80, 255) : CP_Color_Create(50, 150, 50, 255));
        CP_Settings_Stroke(CP_Color_Create(255, 255, 255, 255));
        CP_Settings_StrokeWeight(3);
        CP_Graphics_DrawRect(btn_x, btn_y, btn_w, btn_h);

        // Button text
        CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
        CP_Settings_TextSize(28);
        CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
        CP_Font_DrawText("CONFIRM", btn_x, btn_y);
    }

    // 5. Draw Footer Text
    CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255));
    CP_Settings_TextSize(20);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);

    if (reward_state->reward_claimed) {
        CP_Font_DrawText("Proceeding to next level...", window_width / 2.0f, window_height - 100.0f);
    }
    else if (reward_state->show_confirm_button) {
        CP_Font_DrawText("Click CONFIRM to add to deck", window_width / 2.0f, window_height - 100.0f);
    }
    else {
        CP_Font_DrawText("Click a card to select", window_width / 2.0f, window_height - 100.0f);
    }
}

// Updates player stats and optionally adds cards to the deck
void ApplyRewardSelection(RewardState* reward_state, Deck* deck, Player* player) {
    if (!reward_state || !deck || !player || reward_state->selected_index < 0 ||
        reward_state->selected_index >= reward_state->num_options) {
        return;
    }

    int buff_increase = 2 + player->card_reward_count;
    RewardType selected_type = reward_state->options[reward_state->selected_index].type;
    Card selected_card_template = reward_state->options[reward_state->selected_index].card;

    player->card_reward_count++;

    char new_card_description[200];
    char existing_normal_description[200];
    char existing_special_description[200];

    int new_bonus = 0;
    bool add_cards = false;

    // Logic based on what type was picked
    // We update existing card descriptions in the deck to reflect new stats
    if (selected_type == REWARD_ATTACK_CARD) {
        if (player->attack_bonus == 0) add_cards = true; // Only add Special cards first time
        player->attack_bonus += buff_increase;
        new_bonus = player->attack_bonus;

        snprintf(new_card_description, sizeof(new_card_description), "Cleave:\n%d Dmg (AOE)", 7 + new_bonus);
        snprintf(existing_normal_description, sizeof(existing_normal_description), "Deal %d Dmg.", 7 + new_bonus);

        for (int i = 0; i < deck->size; i++) {
            if (deck->cards[i].type == Attack) {
                if (deck->cards[i].effect == None) {
                    strncpy(deck->cards[i].description, existing_normal_description, sizeof(deck->cards[i].description) - 1);
                }
                else if (deck->cards[i].effect == CLEAVE) {
                    strncpy(deck->cards[i].description, new_card_description, sizeof(deck->cards[i].description) - 1);
                }
            }
        }
    }
    else if (selected_type == REWARD_HEAL_CARD) {
        if (player->heal_bonus == 0) add_cards = true;
        player->heal_bonus += buff_increase;
        new_bonus = player->heal_bonus;

        snprintf(new_card_description, sizeof(new_card_description), "Divine:\nHeal %d\n50%% Dmg (AOE)", 7 + new_bonus);
        snprintf(existing_normal_description, sizeof(existing_normal_description), "Heal %d HP.", 7 + new_bonus);

        for (int i = 0; i < deck->size; i++) {
            if (deck->cards[i].type == Heal) {
                if (deck->cards[i].effect == None) {
                    strncpy(deck->cards[i].description, existing_normal_description, sizeof(deck->cards[i].description) - 1);
                }
                else if (deck->cards[i].effect == DIVINE_STRIKE_EFFECT) {
                    strncpy(deck->cards[i].description, new_card_description, sizeof(deck->cards[i].description) - 1);
                }
            }
        }
    }
    else if (selected_type == REWARD_SHIELD_CARD) {
        if (player->shield_bonus == 0) add_cards = true;
        player->shield_bonus += buff_increase;
        new_bonus = player->shield_bonus;

        snprintf(existing_normal_description, sizeof(existing_normal_description), "Gain %d Shield.", 5 + new_bonus);
        snprintf(existing_special_description, sizeof(existing_special_description), "Bash:\nGain %d Shield\n Damage dealt = Shield", 5 + new_bonus);

        strncpy(new_card_description, existing_special_description, sizeof(new_card_description));

        for (int i = 0; i < deck->size; i++) {
            if (deck->cards[i].type == Shield) {
                if (deck->cards[i].effect == None) {
                    strncpy(deck->cards[i].description, existing_normal_description, sizeof(deck->cards[i].description) - 1);
                }
                else if (deck->cards[i].effect == SHIELD_BASH) {
                    strncpy(deck->cards[i].description, existing_special_description, sizeof(deck->cards[i].description) - 1);
                }
            }
        }
    }

    // If this is the first time selecting this type, add 2 special cards to the deck
    if (add_cards) {
        Card selected_card = selected_card_template;

        CP_Vector deck_pos_topleft = CP_Vector_Set(50, 600);
        selected_card.pos = CP_Vector_Set(
            deck_pos_topleft.x + CARD_W_INIT / 2.0f,
            deck_pos_topleft.y + CARD_H_INIT / 2.0f
        );

        selected_card.target_pos = CP_Vector_Set(0, 0);

        // Ensure correct sizing
        selected_card.card_w = CARD_W_INIT * CARD_SCALE;
        selected_card.card_h = CARD_H_INIT * CARD_SCALE;

        selected_card.is_animating = false;
        selected_card.is_discarding = false;

        strncpy(selected_card.description, new_card_description, sizeof(selected_card.description) - 1);

        AddCardToDeck(deck, selected_card);
        AddCardToDeck(deck, selected_card);
    }
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