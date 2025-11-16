#pragma once
#include "card.h"
#include "deck.h"
#include <stdbool.h>

#define MAX_REWARD_OPTIONS 3

// Reward types
typedef enum {
    REWARD_ATTACK_CARD,
    REWARD_HEAL_CARD,
    REWARD_SHIELD_CARD
} RewardType;

// Reward option struct
typedef struct {
    Card card;
    RewardType type;
    bool is_selected;
} RewardOption;

// Reward state struct
typedef struct {
    RewardOption options[MAX_REWARD_OPTIONS];
    int num_options;
    bool is_active;
    bool reward_claimed;
    int selected_index;
} RewardState;

// Initialize reward system
void InitReward(RewardState* reward_state);

// Generate reward options after clearing a stage
void GenerateRewardOptions(RewardState* reward_state);

// Update reward screen
void UpdateReward(RewardState* reward_state, Deck* deck);

// Skip reward and proceed without adding a card
void SkipReward(RewardState* reward_state);

// Draw reward screen
void DrawReward(RewardState* reward_state);

// Check if a reward option was clicked
bool IsRewardOptionClicked(RewardState* reward_state, int index, float mouse_x, float mouse_y);

// Apply selected reward to the deck
bool ApplyRewardSelection(RewardState* reward_state, Deck* deck);

// Reset reward state
void ResetReward(RewardState* reward_state);

// Check if reward screen is active
bool IsRewardActive(RewardState* reward_state);