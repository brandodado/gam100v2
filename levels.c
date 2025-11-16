#include "levels.h"

// ---------------- Level 1 ----------------
// Tier 1: Basic
Enemy level1_enemies[] = {
    { "Goblin", 20, 20, 5, 5, 0, true, 0, false, false, false, 0 },
    { "Slime",  15, 15, 3, 3, 0, true, 0, false, false, false, 0 }
};
int level1_enemy_count = sizeof(level1_enemies) / sizeof(level1_enemies[0]);

// ---------------- Level 2 ----------------
Enemy level2_enemies[] = {
    { "Orc",   30, 30, 8, 8, 0, true, 0, false, false, false, 0 },
    { "Slime", 15, 15, 3, 3, 0, true, 0, false, false, false, 0 },
    { "Slime", 15, 15, 3, 3, 0, true, 0, false, false, false, 0 }
};
int level2_enemy_count = sizeof(level2_enemies) / sizeof(level2_enemies[0]);

// ---------------- Level 3 (Boss 1) ----------------
// --- MODIFIED: Set enrage to true, enrage_amount to 2 ---
Enemy level3_enemies[] = {
    { "Witch",   70,  70, 8, 8, 5, true, 0, false, false, true, 2 }, // The Enraging Boss
    { "Goblin",  20,  20, 5, 5, 0, true, 0, false, false, false, 0 },
    { "Goblin",  20,  20, 5, 5, 0, true, 0, false, false, false, 0 }
};
int level3_enemy_count = sizeof(level3_enemies) / sizeof(level3_enemies[0]);

// ---------------- Level 4 ----------------
// Tier 2: Stronger Grunts
Enemy level4_enemies[] = {
    { "Orc Grunt",    40, 40, 10, 10, 0, true, 0, false, false, false, 0 },
    { "Goblin",       25, 25, 6, 6, 0, true, 0, false, false, false, 0 },
    { "Orc Grunt",    40, 40, 10, 10, 0, true, 0, false, false, false, 0 }
};
int level4_enemy_count = sizeof(level4_enemies) / sizeof(level4_enemies[0]);

// ---------------- Level 5 ----------------
Enemy level5_enemies[] = {
    { "Armored Goblin", 35, 35, 8, 8, 10, true, 0, false, false, false, 0 },
    { "Orc Grunt",      50, 50, 12, 12, 0, true, 0, false, false, false, 0 },
    { "Armored Goblin", 35, 35, 8, 8, 10, true, 0, false, false, false, 0 }
};
int level5_enemy_count = sizeof(level5_enemies) / sizeof(level5_enemies[0]);

// ---------------- Level 6 (Boss 2) ----------------
// --- MODIFIED: Reduced health from 300 to 220 ---
Enemy level6_enemies[] = {
    { "OGRE WARLORD", 220, 220, 18, 18, 10, true, 0, false, false, true, 3 }
};
int level6_enemy_count = sizeof(level6_enemies) / sizeof(level6_enemies[0]);

// ---------------- Level 7 ----------------
// Tier 3: Elite Enemies
Enemy level7_enemies[] = {
    { "Shadow Stalker", 60, 60, 14, 14, 5, true, 0, false, false, false, 0 },
    { "Orc Shaman",     50, 50, 10, 10, 10, true, 0, false, false, false, 0 },
    { "Shadow Stalker", 60, 60, 14, 14, 5, true, 0, false, false, false, 0 }
};
int level7_enemy_count = sizeof(level7_enemies) / sizeof(level7_enemies[0]);

// ---------------- Level 8 ----------------
Enemy level8_enemies[] = {
    { "Ogre",           90, 90, 16, 16, 0, true, 0, false, false, false, 0 },
    { "Armored Orc",    70, 70, 12, 12, 20, true, 0, false, false, false, 0 },
    { "Ogre",           90, 90, 16, 16, 0, true, 0, false, false, false, 0 }
};
int level8_enemy_count = sizeof(level8_enemies) / sizeof(level8_enemies[0]);

// ---------------- Level 9 (Boss 3) ----------------
// --- MODIFIED: Set enrage to true, enrage_amount to 5 ---
Enemy level9_enemies[] = {
    { "LICH LORD", 400, 400, 25, 25, 15, true, 0, true, false, true, 5 } // Lich is Necro AND Enrages
};
int level9_enemy_count = sizeof(level9_enemies) / sizeof(level9_enemies[0]);