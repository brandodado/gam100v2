#pragma once
#include <stdbool.h>

#ifndef _level_H
#define _level_H

// ---------------- Enemy struct ----------------
typedef struct {
    const char* name;
    int health;
    int max_health;
    int attack;
    int max_attack; // Added for enrage reset
    int shield;
    bool alive;
    int dot_timing;
    bool is_necromancer;
    bool has_used_special;
    bool enrages;
    int enrage_amount; // Added to determine enrage amount
} Enemy;

// ---------------- Level 1 ----------------
extern Enemy level1_enemies[];
extern int level1_enemy_count;

// ---------------- Level 2 ----------------
extern Enemy level2_enemies[];
extern int level2_enemy_count;

// --- NEW: Levels 3-9 ---
// ---------------- Level 3 (Boss) ----------------
extern Enemy level3_enemies[];
extern int level3_enemy_count;

// ---------------- Level 4 ----------------
extern Enemy level4_enemies[];
extern int level4_enemy_count;

// ---------------- Level 5 ----------------
extern Enemy level5_enemies[];
extern int level5_enemy_count;

// ---------------- Level 6 (Boss) ----------------
extern Enemy level6_enemies[];
extern int level6_enemy_count;

// ---------------- Level 7 ----------------
extern Enemy level7_enemies[];
extern int level7_enemy_count;

// ---------------- Level 8 ----------------
extern Enemy level8_enemies[];
extern int level8_enemy_count;

// ---------------- Level 9 (Boss) ----------------
extern Enemy level9_enemies[];
extern int level9_enemy_count;

#endif