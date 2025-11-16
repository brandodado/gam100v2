#pragma once
#include "cprocessing.h"
#include <stdbool.h>

// Forward declaration of Player
// We can now use the typedef name directly
typedef struct Player Player;

// Defines the types of buffs the player can get
typedef enum {
    BUFF_NONE,
    BUFF_LIFESTEAL,     // Heal for 50% of damage dealt
    BUFF_DESPERATE_DRAW, // Draw 3 cards if hand is empty
    BUFF_DIVINE_STRIKE,
    BUFF_SHIELD_BOOST,  // 25% shield buff
    BUFF_ATTACK_UP,     // Increase base Attack

    // --- MODIFIED: Added Lvl 6 Buffs ---
    BUFF_ATTACK_BOOST_35,
    BUFF_HEAL_BOOST_35,
    BUFF_SHIELD_BOOST_35
} BuffType;

// A single buff option to be displayed
typedef struct {
    BuffType type;
    const char* title;
    char description[200]; // MODIFIED: Changed to char[] to allow dynamic text
} BuffOption;

#define MAX_BUFF_OPTIONS 3

// The state of the buff reward screen
typedef struct {
    BuffOption options[MAX_BUFF_OPTIONS];
    int num_options;
    bool is_active;
    bool reward_claimed;
    int selected_index;
    CP_Vector option_pos[MAX_BUFF_OPTIONS];
    float option_w;
    float option_h;
} BuffRewardState;

// --- Function Declarations ---

void InitBuffReward(BuffRewardState* state);
// --- MODIFIED: Added current_level ---
void GenerateBuffOptions(BuffRewardState* state, int current_level);
void UpdateBuffReward(BuffRewardState* state, Player* player);
void DrawBuffReward(BuffRewardState* state);
void ResetBuffReward(BuffRewardState* state);
bool IsBuffRewardActive(BuffRewardState* state);