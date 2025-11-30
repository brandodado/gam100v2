// Main game state definitions, player structure, and core logic declarations.
#pragma once
#include "cprocessing.h"
#include <stdbool.h> 

// Represents the player's stats, buffs, and progress.
typedef struct Player {
    int health;
    int max_health;
    int attack;
    int shield;

    // --- Player Buffs ---
    bool has_lifesteal;
    bool has_desperate_draw;
    bool has_divine_strike;
    bool has_shield_boost;
    bool has_attack_boost_35;
    bool has_heal_boost_35;
    bool has_shield_boost_35;

    int checkpoint_level;
    int death_count;

    // --- Card Bonus Trackers ---
    int attack_bonus;
    int heal_bonus;
    int shield_bonus;
    int card_reward_count;
} Player;

#define MAX_FLOATING_TEXTS 20
typedef struct {
    char text[16];
    CP_Vector pos;
    CP_Color color;
    float timer;
} FloatingText;

#ifndef GAME_H
#define GAME_H

// Loads enemy data and state for the specified level number (1-9).
void LoadLevel(int level);

// Fully resets the player and game state to default values for a new game.
void ResetGame(void);

// Resets transient stage data (particles, timers, selection) but maintains player stats.
void ResetStageState(void);

// Sets the flag indicating whether the game is restarting from a checkpoint (true) or starting fresh (false).
void Game_Set_Restart_Flag(bool value);

// Increments the global death counter for the session.
void Game_Increment_Death_Count(void);

// Returns the total number of times the player has died this session.
int Game_Get_Death_Count(void);

// CProcessing State Functions
void Game_Init(void);
void Game_Update(void);
void Game_Exit(void);

#endif