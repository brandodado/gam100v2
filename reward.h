#pragma once
#include "card.h"
#include "deck.h"
#include "game.h" // <-- MODIFIED: Include game.h to get Player type
#include <stdbool.h>

#define MAX_REWARD_OPTIONS 3

// Reward types
// --- FIX: Added the missing enum definition ---
typedef enum {
    REWARD_ATTACK_CARD,
    REWARD_HEAL_CARD,
    REWARD_SHIELD_CARD
} RewardType;

// Reward option struct
// --- FIX: Added the missing struct definition ---
typedef struct {
    Card card;
    RewardType type;
    bool is_selected;
} RewardOption;

// Reward state struct
// --- FIX: Added the missing struct definition ---
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
// --- MODIFIED: Added Player* ---
void GenerateRewardOptions(RewardState* reward_state, Player* player);

// Update reward screen
// --- MODIFIED: Added Player* ---
void UpdateReward(RewardState* reward_state, Deck* deck, Player* player);

// Skip reward and proceed without adding a card
void SkipReward(RewardState* reward_state);

// Draw reward screen
// --- MODIFIED: Added Player* ---
void DrawReward(RewardState* reward_state, Player* player);

// Check if a reward option was clicked
bool IsRewardOptionClicked(RewardState* reward_state, int index, float mouse_x, float mouse_y);

// Apply selected reward to the deck
// --- MODIFIED: Changed to void return type ---
void ApplyRewardSelection(RewardState* reward_state, Deck* deck, Player* player);

// Reset reward state
void ResetReward(RewardState* reward_state);

// Check if reward screen is active
bool IsRewardActive(RewardState* reward_state);