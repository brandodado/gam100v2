// System for granting new cards to the player after clearing levels.
#pragma once
#include "card.h"
#include "game.h" 
#include <stdbool.h>

#define MAX_REWARD_OPTIONS 3

typedef enum {
    REWARD_ATTACK_CARD,
    REWARD_HEAL_CARD,
    REWARD_SHIELD_CARD
} RewardType;

typedef struct {
    Card card;
    RewardType type;
    bool is_selected;
} RewardOption;

typedef struct {
    RewardOption options[MAX_REWARD_OPTIONS];
    int num_options;
    bool is_active;
    bool reward_claimed;
    bool show_confirm_button;
    int selected_index;
} RewardState;

// Initializes the card reward state.
void InitReward(RewardState* reward_state);

// Generates 3 random or preset card rewards in the reward_state based on player progress.
void GenerateRewardOptions(RewardState* reward_state, Player* player);

// Updates the reward screen logic, handling mouse input to select or confirm rewards.
void UpdateReward(RewardState* reward_state, Deck* deck, Player* player);

// Draws the reward selection screen overlays and card options.
void DrawReward(RewardState* reward_state, Player* player);

// Checks if the reward option at the specified index was clicked by the mouse at (mouse_x, mouse_y).
bool IsRewardOptionClicked(RewardState* reward_state, int index, float mouse_x, float mouse_y);

// Applies the selected reward to the player's deck and closes the screen.
void ApplyRewardSelection(RewardState* reward_state, Deck* deck, Player* player);

// Resets the reward state for the next usage.
void ResetReward(RewardState* reward_state);

// Checks if the reward screen is currently active and visible.
bool IsRewardActive(RewardState* reward_state);