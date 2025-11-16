#include "levels.h"

// ---------------- Level 1 ----------------
// Balanced: 2 Goblins
Enemy level1_enemies[] = {
    { "Goblin", 20, 20, 5, 1, true },
    { "Goblin", 20, 20, 5, 1, true }
};
int level1_enemy_count = sizeof(level1_enemies) / sizeof(level1_enemies[0]);

// ---------------- Level 2 ----------------
// Updated: 1 Orc (Damage), 2 Thieves (Support)
Enemy level2_enemies[] = {
    { "Orc", 12, 12, 12, 2, true },
    { "Goblin Thief", 20, 20, 4, 1, true },
    { "Goblin Thief", 20, 20, 4, 1, true }
};
int level2_enemy_count = sizeof(level2_enemies) / sizeof(level2_enemies[0]);

// ---------------- Level 3 ----------------
// Updated: 1 Orc Brute (Damage), 1 Witch (Support), 1 Thief (Support)
Enemy level3_enemies[] = {
    { "Orc Brute", 30, 30, 25, 2, true},
    { "Witch", 60, 60, 8, 3, true},
    { "Goblin Thief", 20, 20, 4, 1, true}
};
int level3_enemy_count = sizeof(level3_enemies) / sizeof(level3_enemies[0]);