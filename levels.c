#include "levels.h"

// ---------------- Level 1 ----------------
// Tier 1: Basic
Enemy level1_enemies[] = {
    { "Goblin", 21, 21, 5, 5, 0, true, 0, false, false, false, 0 },
    { "Slime",  14, 14, 8, 8, 0, true, 0, false, false, false, 0 }
};
int level1_enemy_count = sizeof(level1_enemies) / sizeof(level1_enemies[0]);

// ---------------- Level 2 ----------------
Enemy level2_enemies[] = {
    { "Goblin",   21, 21, 5, 5, 0, true, 0, false, false, false, 0 },
    { "Slime", 14, 14, 8, 8, 0, true, 0, false, false, false, 0 },
    { "Slime", 14, 14, 8, 8, 0, true, 0, false, false, false, 0 }
};
int level2_enemy_count = sizeof(level2_enemies) / sizeof(level2_enemies[0]);

// ---------------- Level 3 (Boss 1) ----------------
// --- MODIFIED: Set enrage to true, enrage_amount to 2 ---
Enemy level3_enemies[] = {
    { "Witch",   50,  50, 6, 6, 5, true, 0, false, false, true, 2 }, // The Enraging Boss
    { "Orc Grunt",  14,  14, 8, 8, 0, true, 0, false, false, false, 0 },
    { "Goblin",  14,  14, 4, 4, 0, true, 0, false, false, false, 0 }
};
int level3_enemy_count = sizeof(level3_enemies) / sizeof(level3_enemies[0]);

// ---------------- Level 4 ----------------
// Tier 2: Stronger Grunts
Enemy level4_enemies[] = {
    { "Orc Grunt",    28, 28, 8, 8, 0, true, 0, false, false, false, 0 },
    { "Goblin",       21, 21, 4, 4, 0, true, 0, false, false, false, 0 },
    { "Orc Grunt",    28, 28, 8, 8, 0, true, 0, false, false, false, 0 }
};
int level4_enemy_count = sizeof(level4_enemies) / sizeof(level4_enemies[0]);

// ---------------- Level 5 ----------------
Enemy level5_enemies[] = {
    { "Armored Goblin", 35, 35, 6, 6, 10, true, 0, false, false, false, 0 },
    { "Orc Grunt",      42, 42, 8, 8, 0, true, 0, false, false, false, 0 },
    { "Armored Goblin", 35, 35, 6, 6, 10, true, 0, false, false, false, 0 }
};
int level5_enemy_count = sizeof(level5_enemies) / sizeof(level5_enemies[0]);

// ---------------- Level 6 (Boss 2) ----------------
// --- MODIFIED: Reduced health from 300 to 220 ---
Enemy level6_enemies[] = {
    { "OGRE WARLORD", 140, 140, 11, 11, 10, true, 0, false, false, true, 3 }
};
int level6_enemy_count = sizeof(level6_enemies) / sizeof(level6_enemies[0]);

// ---------------- Level 7 ----------------
// Tier 3: Elite Enemies
Enemy level7_enemies[] = {
    { "Shadow Stalker", 40, 40, 8, 8, 5, true, 0, false, false, false, 0 },
    { "Orc Shaman",     30, 30, 6, 6, 10, true, 0, false, false, false, 0 },
    { "Shadow Stalker", 40, 40, 8, 8, 5, true, 0, false, false, false, 0 }
};
int level7_enemy_count = sizeof(level7_enemies) / sizeof(level7_enemies[0]);

// ---------------- Level 8 ----------------
Enemy level8_enemies[] = {
    { "Ogre",           40, 40, 8, 8, 0, true, 0, false, false, false, 0 },
    { "Armored Orc",    60, 60, 14, 14, 20, true, 0, false, false, false, 0 },
    { "Ogre",           40, 40, 8, 8, 0, true, 0, false, false, false, 0 }
};
int level8_enemy_count = sizeof(level8_enemies) / sizeof(level8_enemies[0]);

// ---------------- Level 9 (Boss 3) ----------------
// --- MODIFIED: Set enrage to true, enrage_amount to 4 ---
Enemy level9_enemies[] = {
    { "LICH LORD", 250, 250, 16, 16, 15, true, 0, true, false, true, 4 } // Lich is Necro AND Enrages
};
int level9_enemy_count = sizeof(level9_enemies) / sizeof(level9_enemies[0]);