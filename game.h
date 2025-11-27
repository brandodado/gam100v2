#pragma once
#include "cprocessing.h"
#include <stdbool.h> 

// ---------------- Player Struct ----------------
typedef struct Player {
    int health;
    int max_health;
    int attack;
    int shield;

    // --- Player Buffs ---
    bool has_lifesteal;
    bool has_desperate_draw;
    bool has_divine_strike;
    bool has_shield_boost; // Shield buff

    bool has_attack_boost_35;
    bool has_heal_boost_35;
    bool has_shield_boost_35;

    int checkpoint_level;
    int death_count; // Death counter

    // --- Card Bonus Trackers ---
    int attack_bonus;
    int heal_bonus;
    int shield_bonus;
    int card_reward_count; // Tracks how many times card reward screen has appeared
} Player;

// --- Struct for floating damage/heal text ---
#define MAX_FLOATING_TEXTS 20
typedef struct {
    char text[16];
    CP_Vector pos;
    CP_Color color;
    float timer; // Lifetime in seconds
} FloatingText;

#ifndef GAME_H
#define GAME_H

void LoadLevel(int level);   // Declaration

void ResetGame(void);        // If you call ResetGame from menu

void ResetStageState(void);

// --- Game over functions ---
void Game_Set_Restart_Flag(bool value);
void Game_Increment_Death_Count(void);
int Game_Get_Death_Count(void);

void Game_Init(void);
void Game_Update(void);
void Game_Exit(void);

#endif