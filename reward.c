#define _CRT_SECURE_NO_WARNINGS // <-- ADDED: Suppresses strncpy warnings
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
        reward_font = CP_Font_Load("Assets/Roboto-Regular.ttf");
        font_loaded = true;
    }
}

// --- MODIFIED: Added Player* to generate dynamic descriptions ---
void GenerateRewardOptions(RewardState* reward_state, Player* player) {
    if (!reward_state || !player) return;

    CP_Vector card_pos = CP_Vector_Set(0, 0);
    float card_w = CARD_W_INIT * 1.5f;
    float card_h = CARD_H_INIT * 1.5f;

    // --- NEW: Calculate scaling buff amount ---
    // --- MODIFIED: Reduced scaling formula ---
    int buff_increase = 3 + (player->card_reward_count * 2);

    // --- NEW: Calculate the NEXT bonus level ---
    int next_attack_bonus = player->attack_bonus + buff_increase;
    int next_heal_bonus = player->heal_bonus + buff_increase;
    int next_shield_bonus = player->shield_bonus + buff_increase;

    // --- NEW: Create dynamic descriptions ---
    char attack_desc[200];
    // --- MODIFIED: Updated description text ---
    snprintf(attack_desc, sizeof(attack_desc), "Cleave: Deals %d damage to ALL enemies.\n\nall Attack cards do an additional +%d damage.", 7 + next_attack_bonus, next_attack_bonus);

    char heal_desc[200];
    // --- MODIFIED: Updated description text ---
    snprintf(heal_desc, sizeof(heal_desc), "Heals %d HP. Deals 50%% of heal as AOE damage.\n\nall Heal cards do an additional +%d HP.", 7 + next_heal_bonus, next_heal_bonus);

    char shield_desc[200];

    // --- MODIFIED: Shield Bash is now always offered ---
    CardEffect shield_effect = SHIELD_BASH;
    snprintf(shield_desc, sizeof(shield_desc), "Shield Bash:\nGain %d Shield, then deal damage equal to your new total Shield.\n\nall Shield cards do an additional +%d Shield.", 5 + next_shield_bonus, next_shield_bonus);


    // Create attack card reward
    Card attack_reward = {
        card_pos, card_pos, Attack, CLEAVE, 7, "", // <-- MODIFIED: Set effect to CLEAVE
        card_w, card_h, false, false
    };
    strncpy(attack_reward.description, attack_desc, sizeof(attack_reward.description) - 1);


    // Create heal card reward
    Card heal_reward = {
        card_pos, card_pos, Heal, DIVINE_STRIKE_EFFECT, 7, "", // <-- MODIFIED: Set effect to DIVINE_STRIKE_EFFECT
        card_w, card_h, false, false
    };
    strncpy(heal_reward.description, heal_desc, sizeof(heal_reward.description) - 1);

    // Create shield card reward
    Card shield_reward = {
        card_pos, card_pos, Shield, shield_effect, 5, "",
        card_w, card_h, false, false
    };
    strncpy(shield_reward.description, shield_desc, sizeof(shield_reward.description) - 1);


    // Add all three options
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
}

bool IsRewardOptionClicked(RewardState* reward_state, int index, float mouse_x, float mouse_y) {
    if (!reward_state || index < 0 || index >= reward_state->num_options) {
        return false;
    }

    float window_width = (float)CP_System_GetWindowWidth();
    float card_w = CARD_W_INIT * 1.5f; // Use consistent width
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

// --- MODIFIED: Added Player* to pass to ApplyRewardSelection ---
void UpdateReward(RewardState* reward_state, Deck* deck, Player* player) {
    if (!reward_state || !reward_state->is_active || reward_state->reward_claimed) return;

    // Check for mouse clicks on reward options
    if (CP_Input_MouseClicked()) {
        float mouse_x = (float)CP_Input_GetMouseX();
        float mouse_y = (float)CP_Input_GetMouseY();

        // Check skip button
        float window_width = (float)CP_System_GetWindowWidth();
        float window_height = (float)CP_System_GetWindowHeight();
        float skip_btn_x = window_width / 2.0f;
        float skip_btn_y = (float)window_height - 150.0f;

        if (IsAreaClicked(skip_btn_x, skip_btn_y, SKIP_BUTTON_W, SKIP_BUTTON_H, mouse_x, mouse_y)) {
            SkipReward(reward_state);
            return;
        }

        // Check card options
        for (int i = 0; i < reward_state->num_options; i++) {
            if (IsRewardOptionClicked(reward_state, i, mouse_x, mouse_y)) {
                reward_state->selected_index = i;

                // Apply reward immediately and lock selection
                // --- MODIFIED: Pass 'player' ---
                if (ApplyRewardSelection(reward_state, deck, player)) {
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

// --- MODIFIED: Added Player* to display current bonuses ---
void DrawReward(RewardState* reward_state, Player* player) {
    if (!reward_state || !player || (!reward_state->is_active && !reward_state->reward_claimed)) return;

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
    CP_Font_DrawText("Choose Your Reward!", window_width / 2.0f, 100.0f);

    CP_Settings_RectMode(CP_POSITION_CENTER);

    float card_w = CARD_W_INIT * 1.5f;
    float total_width = reward_state->num_options * card_w +
        (reward_state->num_options - 1) * REWARD_CARD_SPACING;
    float start_x = (window_width - total_width) / 2.0f + (card_w / 2.0f);

    // 3. Draw Cards Loop
    for (int i = 0; i < reward_state->num_options; i++) {
        float card_x = start_x + i * (card_w + REWARD_CARD_SPACING);
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
                continue;
            }
        }
        // B. HANDLE NORMAL SELECTION STATE
        else {
            float mouse_x = (float)CP_Input_GetMouseX();
            float mouse_y = (float)CP_Input_GetMouseY();

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

        DrawCard(&reward_state->options[i].card); // This now draws the dynamic description

        // --- NEW: Draw "Current Bonus" label ---
        CP_Settings_Fill(CP_Color_Create(220, 220, 255, 255));
        CP_Settings_TextSize(18);
        CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);

        char current_bonus_text[100];
        // --- MODIFIED: Simplified shield text ---
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

    // 4. Draw Skip Button
    float skip_btn_x = window_width / 2.0f;
    float skip_btn_y = (float)window_height - 150.0f;
    float mx = (float)CP_Input_GetMouseX();
    float my = (float)CP_Input_GetMouseY();

    if (IsAreaClicked(skip_btn_x, skip_btn_y, SKIP_BUTTON_W, SKIP_BUTTON_H, mx, my)) {
        CP_Settings_Fill(CP_Color_Create(150, 50, 50, 255));
    }
    else {
        CP_Settings_Fill(CP_Color_Create(100, 100, 100, 255));
    }

    CP_Settings_RectMode(CP_POSITION_CENTER);
    CP_Graphics_DrawRect(skip_btn_x, skip_btn_y, SKIP_BUTTON_W, SKIP_BUTTON_H);

    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Settings_TextSize(24);

    // --- FIX: Re-set text alignment to center-middle ---
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);

    CP_Font_DrawText("Skip Reward", skip_btn_x, skip_btn_y); // Centered text

    // 5. Draw Footer Text
    CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255));
    CP_Settings_TextSize(20);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);

    if (reward_state->reward_claimed) {
        CP_Font_DrawText("Proceeding to next level...", window_width / 2.0f, window_height - 100.0f);
    }
    else {
        CP_Font_DrawText("Click a card to add to deck", window_width / 2.0f, window_height - 100.0f);
    }
}

// --- MODIFIED: Added Player* to apply buff and update card descriptions ---
bool ApplyRewardSelection(RewardState* reward_state, Deck* deck, Player* player) {
    if (!reward_state || !deck || !player || reward_state->selected_index < 0 ||
        reward_state->selected_index >= reward_state->num_options) {
        return false;
    }

    // --- FIX: Get buff amount based on the count *before* this reward ---
    // --- MODIFIED: Reduced scaling formula ---
    int buff_increase = 3 + (player->card_reward_count * 2);
    RewardType selected_type = reward_state->options[reward_state->selected_index].type;
    Card selected_card_template = reward_state->options[reward_state->selected_index].card;

    // --- FIX: Increment count *after* applying reward ---
    player->card_reward_count++;

    // --- MODIFIED: Create separate description buffers ---
    char new_card_description[200]; // For the card being added
    char existing_normal_description[200]; // For basic cards already in the deck
    char existing_special_description[200]; // For special cards already in the deck

    int new_bonus = 0;

    if (selected_type == REWARD_ATTACK_CARD) {
        player->attack_bonus += buff_increase;
        new_bonus = player->attack_bonus;

        // Description for the *new* Cleave card being added
        snprintf(new_card_description, sizeof(new_card_description), "Cleave: Deals %d damage to ALL enemies.", 7 + new_bonus);
        // Description for *existing* normal Attack cards
        snprintf(existing_normal_description, sizeof(existing_normal_description), "Deals %d damage.", 7 + new_bonus);

        // Update all cards of this type in the master deck
        for (int i = 0; i < deck->size; i++) {
            if (deck->cards[i].type == Attack) {
                // --- MODIFIED: Update descriptions based on effect ---
                if (deck->cards[i].effect == None) {
                    strncpy(deck->cards[i].description, existing_normal_description, sizeof(deck->cards[i].description) - 1);
                }
                else if (deck->cards[i].effect == CLEAVE) {
                    // Also update existing Cleave cards
                    strncpy(deck->cards[i].description, new_card_description, sizeof(deck->cards[i].description) - 1);
                }
            }
        }
    }
    else if (selected_type == REWARD_HEAL_CARD) {
        player->heal_bonus += buff_increase;
        new_bonus = player->heal_bonus;

        // Description for the *new* Divine Strike card
        snprintf(new_card_description, sizeof(new_card_description), "Heals %d HP. Deals 50%% of heal as AOE damage.", 7 + new_bonus);
        // Description for *existing* normal Heal cards
        snprintf(existing_normal_description, sizeof(existing_normal_description), "Heals %d HP.", 7 + new_bonus);

        // Update all cards of this type in the master deck
        for (int i = 0; i < deck->size; i++) {
            if (deck->cards[i].type == Heal) {
                // --- MODIFIED: Update descriptions based on effect ---
                if (deck->cards[i].effect == None) {
                    strncpy(deck->cards[i].description, existing_normal_description, sizeof(deck->cards[i].description) - 1);
                }
                else if (deck->cards[i].effect == DIVINE_STRIKE_EFFECT) {
                    // Also update existing Divine Strike cards
                    strncpy(deck->cards[i].description, new_card_description, sizeof(deck->cards[i].description) - 1);
                }
            }
        }
    }
    else if (selected_type == REWARD_SHIELD_CARD) {
        player->shield_bonus += buff_increase;
        new_bonus = player->shield_bonus;

        // Description for *normal* shield cards
        snprintf(existing_normal_description, sizeof(existing_normal_description), "Gains %d Shield.", 5 + new_bonus);
        // Description for *Shield Bash* cards
        snprintf(existing_special_description, sizeof(existing_special_description), "Shield Bash:\nGain %d Shield, then deal damage equal to your new total Shield.", 5 + new_bonus);

        // --- MODIFIED: Simplified logic since reward card is *always* Shield Bash ---
        strncpy(new_card_description, existing_special_description, sizeof(new_card_description));

        // Now, update all existing shield cards in the deck
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


    // Get the selected card and reset its position/description for deck
    Card selected_card = selected_card_template; // Use the template

    CP_Vector deck_pos_topleft = CP_Vector_Set(50, 600);
    selected_card.pos = CP_Vector_Set(
        deck_pos_topleft.x + CARD_W_INIT / 2.0f,
        deck_pos_topleft.y + CARD_H_INIT / 2.0f
    );

    selected_card.target_pos = CP_Vector_Set(0, 0);
    selected_card.card_w = CARD_W_INIT;
    selected_card.card_h = CARD_H_INIT;
    selected_card.is_animating = false;
    selected_card.is_discarding = false;

    // --- NEW: Set the description of the *new* card to the updated one ---
    strncpy(selected_card.description, new_card_description, sizeof(selected_card.description) - 1);

    // --- MODIFIED: Add two copies of the card and check success ---
    bool success1 = AddCardToDeck(deck, selected_card);
    bool success2 = AddCardToDeck(deck, selected_card);

    // Return true if *at least one* card was added.
    return success1 || success2;
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