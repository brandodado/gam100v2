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
    int defense;
    bool alive;
    int dot_timing; // <-- NEW: For Damage Over Time
} Enemy;

// ---------------- Level 1 ----------------
extern Enemy level1_enemies[];
extern int level1_enemy_count;

// ---------------- Level 2 ----------------
extern Enemy level2_enemies[];
extern int level2_enemy_count;

// ---------------- Level 3 ----------------
extern Enemy level3_enemies[];
extern int level3_enemy_count;
#endif