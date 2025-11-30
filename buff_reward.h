// System for granting passive buffs to the player after boss levels.
#pragma once
#include "cprocessing.h"
#include <stdbool.h>

typedef struct Player Player;

typedef enum {
    BUFF_NONE,
    BUFF_LIFESTEAL,
    BUFF_DESPERATE_DRAW,
    BUFF_DIVINE_STRIKE,
    BUFF_SHIELD_BOOST,
    BUFF_ATTACK_UP,
    BUFF_ATTACK_BOOST_35,
    BUFF_HEAL_BOOST_35,
    BUFF_SHIELD_BOOST_35
} BuffType;

typedef struct {
    BuffType type;
    const char* title;
    char description[200];
} BuffOption;

#define MAX_BUFF_OPTIONS 3

typedef struct {
    BuffOption options[MAX_BUFF_OPTIONS];
    int num_options;
    bool is_active;
    bool reward_claimed;
    bool show_confirm_button;
    int selected_index;
    CP_Vector option_pos[MAX_BUFF_OPTIONS];
    float option_w;
    float option_h;
} BuffRewardState;

// Initializes the buff reward system.
void InitBuffReward(BuffRewardState* state);

// Generates buff options in the state based on the current_level to determine the boss tier.
void GenerateBuffOptions(BuffRewardState* state, int current_level);

// Updates input and logic for the buff reward screen.
void UpdateBuffReward(BuffRewardState* state, Player* player);

// Renders the buff reward selection screen.
void DrawBuffReward(BuffRewardState* state);

// Resets the buff reward state.
void ResetBuffReward(BuffRewardState* state);

// Checks if the buff reward screen is active.
bool IsBuffRewardActive(BuffRewardState* state);