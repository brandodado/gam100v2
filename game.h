#pragma once
#include <stdbool.h>
#include "cprocessing.h"
// #include "card.h" // <-- REMOVED THIS LINE to fix circular dependency

// ---------------- Player Struct ----------------
typedef struct {
    int health;
    int max_health;
    int attack;
    int defense;
} Player;

// --- Global Card Multiplier ---
extern float g_card_multiplier;

// --- Global Lifesteal Flag ---
extern bool g_player_has_lifesteal;

// --- Global Card Draw Flag ---
extern bool g_player_draw_bonus;

// --- Global Level Tracker ---
extern int current_level;

// --- Global Checkpoint Flag ---
extern bool g_player_has_died;

// --- Global Font ---
extern CP_Font game_font;


void Game_Init(void);

// Declaration for our "checkpoint" init
void Game_Init_At_Level_2_Shop(void);

void Game_Update(void);

void Game_Exit(void);

// Public function to reset checkpoint
void Game_ResetDeathFlag(void);

// Public Game Logic Function
void ExecuteLevelUpChoice(int health_bonus, float multiplier_increase, bool enable_lifesteal, bool enable_draw_bonus);

// Made LoadLevel public so shop.c can call it
void LoadLevel(int level);