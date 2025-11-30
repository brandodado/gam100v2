// @file game.c
// @brief Main gameplay logic, including the game loop, rendering, enemy AI, and card interactions.

#include <math.h>
#include "cprocessing.h"
#include "mainmenu.h"
#include "utils.h"
#include <stdbool.h>
#include "card.h"
#include "levels.h"
#include <stdio.h>
#include "game.h"
#include "reward.h"
#include "gameover.h" 
#include "buff_reward.h" 
#include "victory.h" 
#include <string.h> 
#include "sfx.h"

// ---------------------------------------------------------
// 0. EXTERN DECLARATIONS (Safety Net)
// ---------------------------------------------------------
// Importing enemy data defined in levels.c
extern Enemy level1_enemies[]; extern int level1_enemy_count;
extern Enemy level2_enemies[]; extern int level2_enemy_count;
extern Enemy level3_enemies[]; extern int level3_enemy_count;
extern Enemy level4_enemies[]; extern int level4_enemy_count;
extern Enemy level5_enemies[]; extern int level5_enemy_count;
extern Enemy level6_enemies[]; extern int level6_enemy_count;
extern Enemy level7_enemies[]; extern int level7_enemy_count;
extern Enemy level8_enemies[]; extern int level8_enemy_count;
extern Enemy level9_enemies[]; extern int level9_enemy_count;

// ---------------------------------------------------------
// 1. GLOBAL VARIABLES
// ---------------------------------------------------------

int selected_card_index; // Index of the currently highlighted card in the hand array

#define SELECT_BTN_W 200
#define SELECT_BTN_H 100
#define MAX_LEVEL 9 

#define CARD_SCALE 1.5f

// Screen positions for the draw and discard piles
CP_Vector deck_pos;
CP_Vector discard_pos;

// Turn and Card tracking
int played_cards;
int turn_num;
bool dealt;      // Flag to ensure cards are only dealt once per turn
int developer;   // Debug mode toggle

#define MAX_HAND_SIZE 7      
Card hand[MAX_HAND_SIZE];
Card discard[MAX_DECK_SIZE];
int discard_size;
int hand_size;
int recycling_count;
bool is_recycling; // Flag for the deck shuffling animation

// Main Data Structures
Deck player_deck;
RewardState reward_state;
BuffRewardState buff_reward_state;

// Flag to handle respawning at a checkpoint (set by GameOver screen)
static bool g_is_restarting_from_checkpoint = false;

// --- Player/Game State ---
#define START_ATTACK 7
#define START_SHIELD 0 
#define START_HEALTH 80
#define INITIAL_DECK_SIZE 14

static Player player = {
    START_HEALTH, START_HEALTH, START_ATTACK, START_SHIELD,
    false, false, false, false, // Initial Buff flags (all false)
    false, false, false,        // Tier 2 Buff flags
    1,                          // Checkpoint Level
    0,                          // Death Count
    0, 0, 0, 0                  // Bonus trackers
};

static int selected_enemy = 0; // Index of the currently targeted enemy

// Current level data pointers
static Enemy* current_enemies = NULL;
static int current_enemy_count = 0;
static int current_level = 1;

// UI/Flow Flags
static bool stage_cleared = false;
static float banner_timer = 0.0f;
static bool reward_active = false;
static bool buff_reward_active = false;

// Visual Flash Timers (for hit feedback)
static float player_hit_flash = 0.0f;
static float player_shield_flash = 0.0f;

static float enemy_hit_flash[16] = { 0.0f };
static float enemy_shield_flash[16] = { 0.0f };
static float enemy_slash_timer[16] = { 0.0f }; // Timer for the red slash effect

// --- ASSETS & SPRITES ---
static CP_Image img_player = NULL;
static CP_Image img_goblin = NULL;
static CP_Image img_slime = NULL;
static CP_Image img_witch = NULL;
static CP_Image img_orc = NULL;
static CP_Image img_ogre = NULL;
static CP_Image img_shadow = NULL;
static CP_Image img_lich = NULL;

static CP_Image img_heart_particle = NULL;
static CP_Image img_shield_particle = NULL;

// Floating Text System (Damage Numbers)
static FloatingText floating_texts[MAX_FLOATING_TEXTS];
static int floating_text_count = 0;

// Floating Icon System (Particle Effects)
#define MAX_FLOATING_ICONS 20
typedef struct {
    CP_Vector pos;
    float timer;
    float max_time;
    CP_Image img;
    float scale;
} FloatingIcon;

static FloatingIcon floating_icons[MAX_FLOATING_ICONS];
static int floating_icon_count = 0;

static CP_Font game_font;
static CP_Image game_bg = NULL;

static CP_Sound background_music = NULL;

// --- SFX ASSETS ---
static CP_Sound sfx_shield = NULL;
static CP_Sound sfx_heal = NULL;
static CP_Sound sfx_draw = NULL;

// Battle Phase State Machine
typedef enum {
    PHASE_PLAYER, // Player's turn to play cards
    PHASE_ENEMY   // Enemies attacking
} BattlePhase;

BattlePhase current_phase = PHASE_PLAYER;

// Enemy Turn Animation Variables
float enemy_turn_timer = 0.0f;
int enemy_action_index = 0;      // Which enemy is currently acting
float enemy_anim_offset_x = 0.0f; // For the "lunging" animation
bool enemy_has_hit = false;      // To ensure damage is applied only once per lunge


// ---------------------------------------------------------
// 2. HELPER FUNCTIONS
// ---------------------------------------------------------

// Sets the flag to respawn the player at the start of the current level instead of a fresh game.
void Game_Set_Restart_Flag(bool value) {
    g_is_restarting_from_checkpoint = value;
}

// Tracks the total number of deaths in the session (used for scoring).
void Game_Increment_Death_Count(void) {
    player.death_count++;
}

// Returns the death count for the Victory screen.
int Game_Get_Death_Count(void) {
    return player.death_count;
}

// Spawns a visual particle icon at a specific location.
static void SpawnFloatingIcon(CP_Image img, CP_Vector pos, float scale) {
    if (floating_icon_count >= MAX_FLOATING_ICONS) return;

    FloatingIcon* icon = &floating_icons[floating_icon_count];
    icon->pos = pos;
    icon->img = img;
    icon->scale = scale;
    icon->max_time = 1.5f;
    icon->timer = icon->max_time;
    floating_icon_count++;
}

// Updates physics and rendering for all active floating icons (particles).
static void UpdateAndDrawFloatingIcons(void) {
    float dt = CP_System_GetDt();
    for (int i = 0; i < floating_icon_count; i++) {
        FloatingIcon* icon = &floating_icons[i];
        icon->timer -= dt;

        // Remove expired icons by swapping with the last element
        if (icon->timer <= 0.0f) {
            floating_icons[i] = floating_icons[floating_icon_count - 1];
            floating_icon_count--;
            i--;
            continue;
        }

        // Float upwards
        icon->pos.y -= 30.0f * dt;

        // Fade out
        float alpha_ratio = icon->timer / icon->max_time;
        int alpha = (int)(255.0f * alpha_ratio);
        if (alpha < 0) alpha = 0;

        if (icon->img) {
            CP_Image_Draw(icon->img, icon->pos.x, icon->pos.y,
                CP_Image_GetWidth(icon->img) * icon->scale,
                CP_Image_GetHeight(icon->img) * icon->scale,
                alpha);
        }
    }
}

// Spawns a floating damage number or text message.
static void SpawnFloatingText(const char* text, CP_Vector pos, CP_Color color) {
    if (floating_text_count >= MAX_FLOATING_TEXTS) return;
    FloatingText* ft = &floating_texts[floating_text_count];
    snprintf(ft->text, sizeof(ft->text), "%s", text);
    ft->pos = pos;
    ft->color = color;
    ft->timer = 1.0f;
    floating_text_count++;
}

// Updates physics and rendering for floating text.
static void UpdateAndDrawFloatingText(void) {
    float dt = CP_System_GetDt();
    CP_Settings_TextSize(32);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Font_Set(game_font);
    for (int i = 0; i < floating_text_count; i++) {
        FloatingText* ft = &floating_texts[i];
        ft->timer -= dt;
        if (ft->timer <= 0.0f) {
            floating_texts[i] = floating_texts[floating_text_count - 1];
            floating_text_count--;
            i--;
            continue;
        }
        ft->pos.y -= 20.0f * dt;
        CP_Color color = ft->color;
        color.a = (int)(255.0f * ft->timer); // Fade alpha
        CP_Settings_Fill(color);
        CP_Font_DrawText(ft->text, ft->pos.x, ft->pos.y);
    }
}

// Checks if all enemies in the current level are dead.
static bool AllEnemiesDefeated(void) {
    if (!current_enemies) return false;
    for (int i = 0; i < current_enemy_count; i++) {
        if (current_enemies[i].alive && current_enemies[i].health > 0) {
            return false;
        }
    }
    return true;
}

// Draws the banner text when a stage is cleared.
void DrawStageClearBanner(void) {
    CP_Settings_Fill(CP_Color_Create(255, 255, 0, 255));
    CP_Settings_TextSize(48);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Font_DrawText("STAGE CLEARED!", CP_System_GetWindowWidth() / 2.0f, CP_System_GetWindowHeight() / 2.0f);
}

// Resets temporary variables for the specific combat stage (not player stats).
void ResetStageState(void) {
    selected_enemy = 0;
    for (int i = 0; i < 16; i++) {
        enemy_hit_flash[i] = 0.0f;
        enemy_shield_flash[i] = 0.0f;
        enemy_slash_timer[i] = 0.0f;
    }
    player_hit_flash = 0.0f;
    player_shield_flash = 0.0f;
    floating_text_count = 0;
    floating_icon_count = 0;
    stage_cleared = false;
    reward_active = false;
    buff_reward_active = false;
    banner_timer = 0.0f;
    current_phase = PHASE_PLAYER;
    enemy_anim_offset_x = 0.0f;
    enemy_has_hit = false;
    player.shield = START_SHIELD; // Reset shield at start of new combat
    ResetReward(&reward_state);
    ResetBuffReward(&buff_reward_state);
}

// Completely resets the game to a fresh start state.
void ResetGame(void) {
    player.health = START_HEALTH;
    player.max_health = START_HEALTH;
    player.attack = START_ATTACK;
    player.shield = START_SHIELD;
    // Reset all buffs
    player.has_lifesteal = false;
    player.has_desperate_draw = false;
    player.has_divine_strike = false;
    player.has_shield_boost = false;
    player.has_attack_boost_35 = false;
    player.has_heal_boost_35 = false;
    player.has_shield_boost_35 = false;

    player.checkpoint_level = 1;
    player.death_count = 0;
    player.attack_bonus = 0;
    player.heal_bonus = 0;
    player.shield_bonus = 0;
    player.card_reward_count = 0;

    selected_card_index = -1;
    played_cards = 0;
    dealt = false;
    turn_num = 0;
    current_level = 1;
    current_enemy_count = 0;
    current_enemies = NULL;
    ResetStageState();
}

// Loads specific enemy data for the requested level and resets the deck if needed.
void LoadLevel(int level) {
    // If we pass the max level, go to Victory screen
    if (level > MAX_LEVEL) {
        CP_Engine_SetNextGameState(Victory_Init, Victory_Update, Victory_Exit);
        return;
    }

    player.checkpoint_level = level;
    current_enemies = NULL;
    current_enemy_count = 0;
    current_level = level;

    // Return hand cards to deck and shuffle before starting new level
    if (turn_num > 0) {
        for (int i = 0; i < hand_size; i++) {
            discard[discard_size] = hand[i];
            discard_size++;
        }
        hand_size = 0;
        // Position cards visually in the deck pile
        CP_Vector deck_pos_center = CP_Vector_Set(
            deck_pos.x + (CARD_W_INIT * CARD_SCALE) / 2.0f,
            deck_pos.y + (CARD_H_INIT * CARD_SCALE) / 2.0f
        );
        for (int i = 0; i < discard_size; i++) {
            player_deck.cards[player_deck.size] = discard[i];
            player_deck.cards[player_deck.size].pos = deck_pos_center;
            player_deck.size++;
        }
        discard_size = 0;
        ShuffleDeck(&player_deck);
    }

    dealt = false;
    turn_num = 0;
    played_cards = 0;

    // Select enemy set based on level
    if (level == 1) {
        current_enemies = level1_enemies;
        current_enemy_count = level1_enemy_count;
    }
    else if (level == 2) {
        current_enemies = level2_enemies;
        current_enemy_count = level2_enemy_count;
    }
    else if (level == 3) {
        current_enemies = level3_enemies;
        current_enemy_count = level3_enemy_count;
    }
    else if (level == 4) {
        current_enemies = level4_enemies;
        current_enemy_count = level4_enemy_count;
    }
    else if (level == 5) {
        current_enemies = level5_enemies;
        current_enemy_count = level5_enemy_count;
    }
    else if (level == 6) {
        current_enemies = level6_enemies;
        current_enemy_count = level6_enemy_count;
    }
    else if (level == 7) {
        current_enemies = level7_enemies;
        current_enemy_count = level7_enemy_count;
    }
    else if (level == 8) {
        current_enemies = level8_enemies;
        current_enemy_count = level8_enemy_count;
    }
    else if (level == 9) {
        current_enemies = level9_enemies;
        current_enemy_count = level9_enemy_count;
    }

    // Fallback safety
    if (current_enemies == NULL || current_enemy_count <= 0) {
        current_enemies = level1_enemies;
        current_enemy_count = level1_enemy_count;
    }

    // Reset enemy stats (hp, shield) for the new level
    if (current_enemies && current_enemy_count > 0) {
        for (int i = 0; i < current_enemy_count; i++) {
            current_enemies[i].health = current_enemies[i].max_health;
            current_enemies[i].shield = 0;
            current_enemies[i].alive = true;
            current_enemies[i].has_used_special = false;
            // Reset enrage status if applicable
            if (current_enemies[i].enrages) {
                current_enemies[i].attack = current_enemies[i].max_attack;
            }
        }
    }

    ResetStageState();
}

// Logic for cycling through targetable enemies using Left/Right keys.
static void HandleEnemySelection(void) {
    if (!current_enemies || current_enemy_count == 0) {
        selected_enemy = -1;
        return;
    }
    int current_selection = selected_enemy;
    int original_selection = selected_enemy;
    bool found_new_target = false;

    // Ensure current selection is valid; if not, find the first alive enemy
    if (original_selection < 0 || original_selection >= current_enemy_count || (current_enemies && !current_enemies[original_selection].alive)) {
        for (int i = 0; i < current_enemy_count; ++i) {
            if (current_enemies[i].alive) {
                selected_enemy = i;
                found_new_target = true;
                break;
            }
        }
        if (found_new_target) return;
        else {
            selected_enemy = -1;
            return;
        }
    }

    // Cycle Left
    if (CP_Input_KeyTriggered(KEY_LEFT)) {
        current_selection--;
        while (current_selection != original_selection) {
            if (current_selection < 0) current_selection = current_enemy_count - 1;
            if (current_enemies[current_selection].alive) {
                selected_enemy = current_selection;
                return;
            }
            if (current_selection == original_selection) break;
            current_selection--;
        }
    }
    // Cycle Right
    if (CP_Input_KeyTriggered(KEY_RIGHT)) {
        current_selection++;
        while (current_selection != original_selection) {
            if (current_selection >= current_enemy_count) current_selection = 0;
            if (current_enemies[current_selection].alive) {
                selected_enemy = current_selection;
                return;
            }
            if (current_selection == original_selection) break;
            current_selection++;
        }
    }
}

// Checks if stage is cleared and manages the Reward Screen transition.
static int UpdateStageClear(void) {
    // Detect if all enemies died just now
    if (!stage_cleared && !reward_active && !buff_reward_active && AllEnemiesDefeated() && current_enemy_count > 0) {
        stage_cleared = true;
        banner_timer = 2.0f;
    }

    if (stage_cleared) {
        if (banner_timer > 0.0f) {
            // Show "Stage Cleared" banner for a few seconds
            banner_timer -= CP_System_GetDt();
            DrawStageClearBanner();
            UpdateAndDrawFloatingText();
            UpdateAndDrawFloatingIcons();
            return 1; // Block other updates
        }
        else {
            // After banner, trigger reward generation
            stage_cleared = false;
            // Boss Levels (3, 6, 9) get Buff Rewards
            if (current_level == 3 || current_level == 6 || current_level == 9) {
                buff_reward_active = true;
                GenerateBuffOptions(&buff_reward_state, current_level);
            }
            else {
                // Normal Levels get Card Rewards
                reward_active = true;
                GenerateRewardOptions(&reward_state, &player);
            }
            return 1;
        }
    }

    // Handle Reward Screen Logic
    if (reward_active) {
        UpdateReward(&reward_state, &player_deck, &player);
        DrawReward(&reward_state, &player);
        if (reward_state.reward_claimed) {
            reward_active = false;
            LoadLevel(current_level + 1);
        }
        return 1;
    }

    // Handle Buff Reward Screen Logic
    if (buff_reward_active) {
        UpdateBuffReward(&buff_reward_state, &player);
        DrawBuffReward(&buff_reward_state);
        if (buff_reward_state.reward_claimed) {
            buff_reward_active = false;
            LoadLevel(current_level + 1);
        }
        return 1;
    }
    return 0; // Normal game update can proceed
}

// General function to render a character (Player or Enemy) with health bars and status effects.
static void DrawEntity(const char* name, int health, int max_health,
    int attack, int shield, float x, float y, float w, float h,
    CP_Color model_color, int is_selected, float hit_flash_timer, float shield_flash_timer, float slash_timer)
{
    // Lazy load sprites if needed
    if (img_player == NULL) img_player = CP_Image_Load("Assets/player.png");
    if (img_goblin == NULL) img_goblin = CP_Image_Load("Assets/goblin.png");
    if (img_slime == NULL)  img_slime = CP_Image_Load("Assets/slime.png");
    if (img_witch == NULL)  img_witch = CP_Image_Load("Assets/witch.png");
    if (img_orc == NULL)    img_orc = CP_Image_Load("Assets/orc.png");
    if (img_ogre == NULL)   img_ogre = CP_Image_Load("Assets/ogre.png");
    if (img_shadow == NULL) img_shadow = CP_Image_Load("Assets/shadow.png");
    if (img_lich == NULL)   img_lich = CP_Image_Load("Assets/lich.png");
    if (img_heart_particle == NULL) img_heart_particle = CP_Image_Load("Assets/icon_heart.png");
    if (img_shield_particle == NULL) img_shield_particle = CP_Image_Load("Assets/icon_shield.png");

    int clamped_health = (health < 0) ? 0 : health;
    if (clamped_health > max_health) clamped_health = max_health;

    float ratio = (max_health > 0) ? ((float)clamped_health / (float)max_health) : 0.0f;

    CP_Settings_RectMode(CP_POSITION_CORNER);

    // Draw Selection Highlight
    if (is_selected) {
        CP_Settings_StrokeWeight(4.0f);
        CP_Settings_Stroke(CP_Color_Create(255, 255, 0, 255));
        CP_Graphics_DrawRect(x - 4.0f, y - 4.0f, w + 8.0f, h + 8.0f);
    }

    // Draw Hit/Shield Flash effects
    if (hit_flash_timer > 0.0f) {
        CP_Settings_Fill(CP_Color_Create(255, 0, 0, 150 + (int)(hit_flash_timer * 105.0f)));
        CP_Graphics_DrawRect(x - 8.0f, y - 8.0f, w + 16.0f, h + 16.0f);
    }
    else if (shield_flash_timer > 0.0f) {
        CP_Settings_Fill(CP_Color_Create(0, 150, 255, 150 + (int)(shield_flash_timer * 105.0f)));
        CP_Graphics_DrawRect(x - 8.0f, y - 8.0f, w + 16.0f, h + 16.0f);
    }
    CP_Settings_StrokeWeight(0);

    // Draw Sprite based on name
    CP_Image current_sprite = NULL;
    if (strcmp(name, "Player") == 0) current_sprite = img_player;
    else if (strstr(name, "Goblin") != NULL) current_sprite = img_goblin;
    else if (strstr(name, "Slime") != NULL)  current_sprite = img_slime;
    else if (strstr(name, "Witch") != NULL)  current_sprite = img_witch;
    else if (strstr(name, "Orc") != NULL)    current_sprite = img_orc;
    else if (strstr(name, "Ogre") != NULL)   current_sprite = img_ogre;
    else if (strstr(name, "OGRE") != NULL)   current_sprite = img_ogre;
    else if (strstr(name, "Shadow") != NULL) current_sprite = img_shadow;
    else if (strstr(name, "LICH") != NULL)   current_sprite = img_lich;
    else if (strstr(name, "Lich") != NULL)   current_sprite = img_lich;

    if (current_sprite != NULL) {
        CP_Image_Draw(current_sprite, x + w / 2.0f, y + h / 2.0f, w, h, 255);
    }
    else {
        // Fallback rectangle
        CP_Settings_Fill(model_color);
        CP_Graphics_DrawRect(x, y, w, h);
    }

    // Draw Slash effect (visual line across entity)
    if (slash_timer > 0.0f) {
        float max_slash_time = 0.3f;
        float alpha_ratio = slash_timer / max_slash_time;
        int alpha = (int)(255 * alpha_ratio);
        if (alpha > 255) alpha = 255;
        if (alpha < 0) alpha = 0;
        CP_Settings_StrokeWeight(6.0f);
        CP_Settings_Stroke(CP_Color_Create(255, 0, 0, alpha));
        CP_Graphics_DrawLine(x, y, x + w, y + h);
        CP_Settings_StrokeWeight(0);
    }

    // Draw Health Bar Background
    float bar_h = 16.0f;
    float bar_y = y + h + 4.0f;
    CP_Settings_Fill(CP_Color_Create(70, 70, 70, 255));
    CP_Graphics_DrawRect(x, bar_y, w, bar_h);

    // Draw Health Bar Foreground (Color changes based on health %)
    CP_Color hp_color = (ratio > 0.6f) ? CP_Color_Create(0, 200, 0, 255)
        : (ratio > 0.3f) ? CP_Color_Create(255, 200, 0, 255)
        : CP_Color_Create(200, 0, 0, 255);
    CP_Settings_Fill(hp_color);
    CP_Graphics_DrawRect(x, bar_y, w * ratio, bar_h);

    // Draw Name
    CP_Font_Set(game_font);
    CP_Settings_TextSize(20);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Font_DrawText(name, x + w * 0.5f, y - 20.0f);

    // Draw HP Text (e.g., 20/20)
    char hp_text[32];
    snprintf(hp_text, sizeof(hp_text), "%d/%d", clamped_health, max_health);
    CP_Settings_TextSize(16);
    CP_Font_DrawText(hp_text, x + w * 0.5f, bar_y + bar_h * 0.5f + 2.0f);

    // Draw Stats (Attack, Shield)
    CP_Settings_TextSize(18);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
    float stats_y = bar_y + bar_h + 10.0f;

    CP_Settings_Fill(CP_Color_Create(255, 80, 80, 255));
    char atk_text[32]; snprintf(atk_text, sizeof(atk_text), "ATK: %d", attack);
    CP_Font_DrawText(atk_text, x, stats_y);

    if (shield > 0) {
        CP_Settings_Fill(CP_Color_Create(80, 120, 255, 255));
        char shield_text[32]; snprintf(shield_text, sizeof(shield_text), "SHD: %d", shield);
        CP_Font_DrawText(shield_text, x + (w * 0.5f) + 10.0f, stats_y);
    }
}

// Manages the enemy turn sequence: Animation -> Damage Calculation -> Next Enemy.
void UpdateEnemyTurn(void) {
    if (!current_enemies) {
        current_phase = PHASE_PLAYER;
        return;
    }
    float dt = CP_System_GetDt();
    enemy_turn_timer += dt;

    // Check if all enemies have acted
    if (enemy_action_index >= current_enemy_count) {
        // End Enemy Phase, Start Player Phase
        current_phase = PHASE_PLAYER;
        turn_num++;
        played_cards = 0;
        dealt = false;
        enemy_anim_offset_x = 0.0f;
        enemy_has_hit = false;

        // Handle Enrage Mechanic (Bosses gain ATK every turn)
        if (current_enemies) {
            for (int i = 0; i < current_enemy_count; i++) {
                if (current_enemies[i].alive && current_enemies[i].enrages) {
                    current_enemies[i].attack += current_enemies[i].enrage_amount;
                    // Visual feedback for enrage
                    float ww = (float)CP_System_GetWindowWidth();
                    float wh = (float)CP_System_GetWindowHeight();
                    float enemy_width = 120.0f;
                    float spacing = 40.0f;
                    float total_width = (float)current_enemy_count * enemy_width + ((float)current_enemy_count - 1.0f) * spacing;
                    float start_x = ww - total_width - 200.0f;
                    float enemy_x = start_x + (float)i * (enemy_width + spacing);
                    float enemy_y = wh / 2.0f - 160.0f / 2.0f;
                    char enrage_text[16];
                    snprintf(enrage_text, sizeof(enrage_text), "ATK +%d", current_enemies[i].enrage_amount);
                    SpawnFloatingText(enrage_text, CP_Vector_Set(enemy_x + 60.0f, enemy_y), CP_Color_Create(255, 100, 100, 255));
                }
            }
        }
        return;
    }

    // Get current acting enemy
    Enemy* e = &current_enemies[enemy_action_index];
    if (!e->alive) {
        enemy_action_index++;
        enemy_turn_timer = 0.0f;
        return;
    }

    // 1. Move Forward (Lunge)
    if (enemy_turn_timer < 0.3f) {
        float ratio = enemy_turn_timer / 0.3f;
        enemy_anim_offset_x = -200.0f * ratio;
    }
    // 2. Deal Damage (Mid-animation)
    else if (!enemy_has_hit) {
        enemy_has_hit = true;
        enemy_anim_offset_x = -200.0f;
        int damage_to_deal = e->attack;
        int damage_blocked = 0;
        int damage_dealt = 0;
        // Apply Shield Mitigation
        if (player.shield > 0) {
            damage_blocked = (damage_to_deal <= player.shield) ? damage_to_deal : player.shield;
            player.shield -= damage_blocked;
            player_shield_flash = 0.2f;
        }
        damage_dealt = damage_to_deal - damage_blocked;

        // Apply Damage to Health
        if (damage_dealt > 0) {
            player.health -= damage_dealt;
            player_hit_flash = 0.2f;
            char text[16];
            snprintf(text, sizeof(text), "-%d", damage_dealt);
            SpawnFloatingText(text, CP_Vector_Set(175.0f, CP_System_GetWindowHeight() / 2.0f), CP_Color_Create(255, 80, 80, 255));
        }
        else {
            SpawnFloatingText("Block!", CP_Vector_Set(175.0f, CP_System_GetWindowHeight() / 2.0f), CP_Color_Create(150, 150, 255, 255));
        }
    }
    // 3. Move Back
    else if (enemy_turn_timer < 0.6f) {
        float ratio = (enemy_turn_timer - 0.3f) / 0.3f;
        enemy_anim_offset_x = -200.0f * (1.0f - ratio);
    }
    // 4. End Turn for this Enemy
    else {
        enemy_action_index++;
        enemy_turn_timer = 0.0f;
        enemy_anim_offset_x = 0.0f;
        enemy_has_hit = false;
    }
}

// ---------------------------------------------------------
// 3. INITIALIZATION
// ---------------------------------------------------------

// Called once when the game state starts. Loads assets and sets up the board.
void Game_Init(void)
{
    game_bg = CP_Image_Load("Assets/background.png");

    if (background_music == NULL)
    {
        background_music = CP_Sound_LoadMusic("Assets/background_music.wav");

        if (background_music == NULL) {
            printf("Error: Could not load Assets/background_music.wav\n");
            CP_Engine_Terminate();
        }
    }

    Audio_Init();

    // Load SFX
    sfx_shield = CP_Sound_Load("Assets/shield_sfx.ogg");
    sfx_heal = CP_Sound_Load("Assets/heal_sfx.ogg");
    sfx_draw = CP_Sound_Load("Assets/card_in.ogg");

    // 2. Play the music ONCE here, not in Update
    // This function usually loops music automatically
    if (background_music != NULL) {
        CP_Sound_PlayMusic(background_music);
    }
    game_font = CP_Font_Load("Assets/Exo2-Regular.ttf");
    if (game_font != 0) { CP_Font_Set(game_font); CP_Settings_TextSize(24); }

    deck_pos = CP_Vector_Set(50, 550);
    discard_pos = CP_Vector_Set(250, 550);

    InitReward(&reward_state);
    InitBuffReward(&buff_reward_state);

    // Check if we are restarting from a death (Checkpoint) or a fresh game
    if (g_is_restarting_from_checkpoint) {
        g_is_restarting_from_checkpoint = false;
        // Restore health but keep buffs/progress
        player.health = player.max_health;
        player.shield = START_SHIELD;
        hand_size = 0;
        discard_size = 0;
        dealt = false;
        turn_num = 0;
        played_cards = 0;
        selected_card_index = -1;
        recycling_count = 0;
        is_recycling = false;
        LoadLevel(player.checkpoint_level);
    }
    else {
        // Reset everything
        ResetGame();
        InitDeck(&player_deck);
        ShuffleDeck(&player_deck);
        LoadLevel(current_level);
    }
}

// Counts cards in hand that aren't currently being animated/discarded.
static int GetActiveHandSize(void) {
    int count = 0;
    for (int i = 0; i < hand_size; i++) {
        if (!hand[i].is_discarding) {
            count++;
        }
    }
    return count;
}

// ---------------------------------------------------------
// 4. GAME UPDATE LOOP
// ---------------------------------------------------------

// Main loop called every frame. Handles Logic, Input, and Rendering.
void Game_Update(void) {
    float dt = CP_System_GetDt();
    float ww = (float)CP_System_GetWindowWidth();
    float wh = (float)CP_System_GetWindowHeight();
    float mx = (float)CP_Input_GetMouseX();
    float my = (float)CP_Input_GetMouseY();
    int mouse_clicked = CP_Input_MouseClicked();
    bool hand_needs_realignment = false;

    // 1. Background    
    CP_Graphics_ClearBackground(CP_Color_Create(20, 25, 28, 255));
    if (game_bg) CP_Image_Draw(game_bg, ww * 0.5f, wh * 0.5f, ww, wh, 255);

    // 2. Overlays & Game State Checks
    // If showing a banner or reward screen, block normal gameplay
    if (UpdateStageClear() == 1) {
        // Ensure hand is cleared into deck for next level
        for (int i = 0; i < hand_size; i++) {
            discard[discard_size] = hand[i];
            discard_size++;
        }
        hand_size = 0;
        RecycleDeck(discard, &player_deck, &discard_size);

        return;
    }

    // Check for Game Over
    if (player.health <= 0) {
        for (int i = 0; i < hand_size; i++) {
            discard[discard_size] = hand[i];
            discard_size++;
        }
        hand_size = 0;
        RecycleDeck(discard, &player_deck, &discard_size);
        CP_Engine_SetNextGameState(GameOver_Init, GameOver_Update, GameOver_Exit);
        return;
    }

    // 3. Update Timers (Flash effects)
    if (player_hit_flash > 0.0f) player_hit_flash -= dt;
    if (player_shield_flash > 0.0f) player_shield_flash -= dt;
    if (current_enemies) {
        for (int i = 0; i < current_enemy_count; i++) {
            if (enemy_hit_flash[i] > 0.0f) enemy_hit_flash[i] -= dt;
            if (enemy_shield_flash[i] > 0.0f) enemy_shield_flash[i] -= dt;
            if (enemy_slash_timer[i] > 0.0f) enemy_slash_timer[i] -= dt;
        }
    }

    // 4. Phase Logic
    if (current_phase == PHASE_ENEMY) {
        UpdateEnemyTurn();
    }
    else {
        // Player Turn: Update targeting logic
        int living_enemies = 0;
        if (current_enemies) {
            for (int i = 0; i < current_enemy_count; i++) {
                if (current_enemies[i].alive) {
                    living_enemies++;
                }
            }
        }
        // Auto-select valid enemy if current target is dead or invalid
        if (selected_enemy == -1 || (current_enemies && !current_enemies[selected_enemy].alive)) {
            HandleEnemySelection();
        }
        else if (living_enemies > 1) {
            HandleEnemySelection();
        }
    }

    // 5. Render Player
    DrawEntity("Player", player.health, player.max_health,
        player.attack, player.shield,
        100.0f, wh / 2.0f - 100.0f, 150.0f, 200.0f,
        CP_Color_Create(50, 50, 150, 255),
        0,
        player_hit_flash, player_shield_flash, 0.0f);

    // 6. Render Enemies
    if (current_enemies && current_enemy_count > 0) {
        float enemy_width = 120.0f;
        float enemy_height = 160.0f;
        float spacing = 40.0f;
        float start_x = ww - (float)current_enemy_count * enemy_width - 200.0f;
        float y = wh / 2.0f - enemy_height / 2.0f;

        for (int i = 0; i < current_enemy_count; i++) {
            Enemy* e = &current_enemies[i];
            float x = start_x + i * (enemy_width + spacing);
            // Apply lunge animation offset
            if (current_phase == PHASE_ENEMY && i == enemy_action_index && e->alive) {
                x += enemy_anim_offset_x;
            }
            CP_Color col = e->alive ? CP_Color_Create(120, 120, 120, 255) : CP_Color_Create(80, 80, 80, 150);
            DrawEntity(e->name, e->health, e->max_health, e->attack, e->shield,
                x, y, enemy_width, enemy_height, col,
                (selected_enemy == i), enemy_hit_flash[i], enemy_shield_flash[i], enemy_slash_timer[i]);
        }
    }

    // 7. Draw HUD (Text overlays)
    char hud_text[128];
    snprintf(hud_text, sizeof(hud_text), "Level %d | Turn %d", current_level, turn_num + 1);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
    CP_Settings_Fill(CP_Color_Create(255, 255, 0, 255));
    CP_Font_Set(game_font);
    CP_Settings_TextSize(30);
    CP_Font_DrawText(hud_text, 20, 35);

    char death_text[32];
    snprintf(death_text, sizeof(death_text), "Restarts: %d", player.death_count);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_RIGHT, CP_TEXT_ALIGN_V_TOP);
    CP_Font_DrawText(death_text, ww - 20.0f, 35.0f);

    // Draw Buff List (Active Passive Effects)
    float buff_text_y = 70.0f;
    CP_Settings_TextSize(18);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
    CP_Settings_Fill(CP_Color_Create(0, 255, 255, 255));
    if (player.has_lifesteal) { CP_Font_DrawText("Buff: Vampiric Strike (50% Lifesteal)", 20, buff_text_y); buff_text_y += 25.0f; }
    if (player.has_desperate_draw) { CP_Font_DrawText("Buff: Card Mastery (+1 Card Draw)", 20, buff_text_y); buff_text_y += 25.0f; }
    if (player.has_divine_strike) { CP_Font_DrawText("Buff: Divine Strike (Heal deals 50% AOE damage)", 20, buff_text_y); buff_text_y += 25.0f; }
    if (player.has_shield_boost) { CP_Font_DrawText("Buff: Reinforce (25% Bonus Shield)", 20, buff_text_y); buff_text_y += 25.0f; }
    if (player.has_attack_boost_35) { CP_Font_DrawText("Buff: Power Infusion (35% Bonus Attack)", 20, buff_text_y); buff_text_y += 25.0f; }
    if (player.has_heal_boost_35) { CP_Font_DrawText("Buff: Holy Infusion (35% Bonus Heal)", 20, buff_text_y); buff_text_y += 25.0f; }
    if (player.has_shield_boost_35) { CP_Font_DrawText("Buff: Barrier Infusion (35% Bonus Shield)", 20, buff_text_y); buff_text_y += 25.0f; }

    UpdateAndDrawFloatingText();
    UpdateAndDrawFloatingIcons();

    // 8. Card Logic (Discarding & Cleanup)
    for (int i = 0; i < hand_size; i++) {
        // Move card from hand array to discard array if it is discarding
        if (hand[i].is_discarding && !hand[i].is_animating) {
            discard[discard_size] = hand[i];
            discard[discard_size].is_discarding = false;
            discard_size++;
            // Shift remaining cards down
            for (int j = i; j < hand_size - 1; j++) {
                hand[j] = hand[j + 1];
            }
            hand_size--;
            hand_needs_realignment = true;
            i--;
        }
    }
    // Re-calculate card positions if cards were removed
    if (hand_needs_realignment) {
        SetHandPos(hand, hand_size);
        if (selected_card_index >= hand_size) {
            selected_card_index = hand_size - 1;
        }
        if (selected_card_index < 0 && hand_size > 0) {
            selected_card_index = 0;
        }
    }

    // 9. Draw Deck Piles
    CP_Settings_RectMode(CP_POSITION_CORNER);
    CP_Settings_Stroke(CP_Color_Create(0, 0, 0, 255));
    CP_Settings_Fill(CP_Color_Create(145, 145, 145, 255));
    CP_Graphics_DrawRect(deck_pos.x, deck_pos.y, CARD_W_INIT * CARD_SCALE, CARD_H_INIT * CARD_SCALE);
    CP_Graphics_DrawRect(discard_pos.x, discard_pos.y, CARD_W_INIT * CARD_SCALE, CARD_H_INIT * CARD_SCALE);

    CP_Font_Set(game_font);
    CP_Settings_TextSize(24);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Font_DrawText("Draw Pile", deck_pos.x + ((CARD_W_INIT * CARD_SCALE) / 2.0f), deck_pos.y - 30.0f);
    CP_Font_DrawText("Discard", discard_pos.x + ((CARD_W_INIT * CARD_SCALE) / 2.0f), discard_pos.y - 30.0f);

    char count_text[16];
    snprintf(count_text, sizeof(count_text), "%d", player_deck.size);
    CP_Font_DrawText(count_text, deck_pos.x + ((CARD_W_INIT * CARD_SCALE) / 2.0f), deck_pos.y + ((CARD_H_INIT * CARD_SCALE) / 2.0f));
    snprintf(count_text, sizeof(count_text), "%d", discard_size);
    CP_Font_DrawText(count_text, discard_pos.x + ((CARD_W_INIT * CARD_SCALE) / 2.0f), discard_pos.y + ((CARD_H_INIT * CARD_SCALE) / 2.0f));

    // 10. Deal Cards (Start of Turn)
    if (!dealt) {
        int cards_to_draw = 4;
        if (player.has_desperate_draw) {
            cards_to_draw = 5; // Draw 1 extra card with buff
        }
        // deal the cards
        for (int i = 0; i < cards_to_draw && player_deck.size > 0; i++) {
            DealFromDeck(&player_deck, &hand[hand_size], &hand_size);
            CP_Sound_Play(sfx_draw); // Play draw sound for each card
        }

        // set their hand position
        SetHandPos(hand, hand_size);
        // change flag
        dealt = true;

        if (hand_size > 0) selected_card_index = 0;
        else selected_card_index = -1;
    }

    // 11. Draw Hand Cards
    for (int i = 0; i < hand_size; ++i) {
        if (hand[i].is_animating) AnimateMoveCard(&hand[i], 900);

        // Highlight selected card
        if (i == selected_card_index && !hand[i].is_discarding) {
            CP_Settings_Fill(CP_Color_Create(0, 0, 0, 0));
            CP_Settings_Stroke(CP_Color_Create(255, 255, 0, 255));
            CP_Settings_StrokeWeight(5.0f);
            CP_Settings_RectMode(CP_POSITION_CENTER);
            CP_Graphics_DrawRect(hand[i].pos.x, hand[i].pos.y, hand[i].card_w + 5.0f, hand[i].card_h + 5.0f);
        }

        DrawCard(&hand[i]);
    }

    // Highlight target (player or enemy) based on selected card type
    if (selected_card_index >= 0 && selected_card_index < hand_size && !hand[selected_card_index].is_discarding) {
        Card* selected = &hand[selected_card_index];

        if (selected->type == Heal || selected->type == Shield) {
            // Highlight player (Self-targeting)
            CP_Settings_Fill(CP_Color_Create(0, 0, 0, 0));
            CP_Settings_Stroke(CP_Color_Create(0, 255, 0, 255)); // Green highlight
            CP_Settings_StrokeWeight(4.0f);
            CP_Settings_RectMode(CP_POSITION_CORNER);
            CP_Graphics_DrawRect(100.0f - 4.0f, wh / 2.0f - 100.0f - 4.0f, 150.0f + 8.0f, 200.0f + 8.0f);
        }
        else if (selected->type == Attack && current_enemies && selected_enemy >= 0 && selected_enemy < current_enemy_count) {
            // Enemy highlight is already dealt in DrawEntity function
        }
    }

    // 12. Action Button (End Turn / Use Card)
    float select_btn_x = ww - 150.0f;
    float select_btn_y = wh - 100.0f;
    if (selected_card_index >= 0 || played_cards > 0 || hand_size > 0) {
        CP_Settings_RectMode(CP_POSITION_CENTER);
        CP_Settings_Fill(CP_Color_Create(100, 100, 100, 255));
        CP_Graphics_DrawRect(select_btn_x, select_btn_y, SELECT_BTN_W, SELECT_BTN_H);

        CP_Font_Set(game_font);
        CP_Settings_TextSize(32);
        CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
        CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));

        if (selected_card_index >= 0) {
            CP_Font_DrawText("Use Card (S)", select_btn_x, select_btn_y);
        }
        else {
            CP_Font_DrawText("End Turn (Enter)", select_btn_x, select_btn_y);
        }
    }

    // 13. Input Logic (Mouse & Keyboard)
    bool card_played_this_frame = false;
    bool clicked_on_card_in_hand = false;
    if (current_phase == PHASE_PLAYER && mouse_clicked) {
        // Check click on Cards
        for (int i = 0; i < hand_size; ++i) {
            if (hand[i].is_discarding) continue;
            if (IsAreaClicked(hand[i].pos.x, hand[i].pos.y, hand[i].card_w, hand[i].card_h, mx, my)) {
                SelectCard(i, &selected_card_index);
                clicked_on_card_in_hand = true;
                break;
            }
        }
        // Check click on Player (to use Heal/Shield)
        if (!clicked_on_card_in_hand && selected_card_index >= 0 && (hand[selected_card_index].type == Heal || hand[selected_card_index].type == Shield)) {
            float player_center_x = 100.0f + 150.0f / 2.0f;
            float player_center_y = (wh / 2.0f - 100.0f) + 200.0f / 2.0f;
            if (IsAreaClicked(player_center_x, player_center_y, 150.0f, 200.0f, mx, my)) card_played_this_frame = true;
        }
        // Check click on Enemy (to use Attack)
        if (!clicked_on_card_in_hand && selected_card_index >= 0 && (hand[selected_card_index].type == Attack)) {
            if (current_enemies) {
                float enemy_width = 120.0f;
                float spacing = 40.0f;
                float start_x = ww - (float)current_enemy_count * enemy_width - 200.0f;
                float enemy_y_center = (wh / 2.0f - 160.0f / 2.0f) + 160.0f / 2.0f;
                for (int i = 0; i < current_enemy_count; i++) {
                    float enemy_x_center = (start_x + i * (enemy_width + spacing)) + enemy_width / 2.0f;
                    if (current_enemies[i].alive && IsAreaClicked(enemy_x_center, enemy_y_center, enemy_width, 160.0f, mx, my)) {
                        selected_enemy = i;
                        card_played_this_frame = true;
                        break;
                    }
                }
            }
        }
        // Check click on Button
        if (!clicked_on_card_in_hand && !card_played_this_frame && IsAreaClicked(select_btn_x, select_btn_y, SELECT_BTN_W, SELECT_BTN_H, mx, my)) {
            if (selected_card_index >= 0) card_played_this_frame = true;
            else {
                // End Turn Button
                current_phase = PHASE_ENEMY;
                enemy_action_index = 0;
                enemy_turn_timer = 0.0f;
                selected_card_index = -1;
                // Discard all remaining hand cards
                for (int i = 0; i < hand_size; i++) {
                    hand[i].is_discarding = true;
                    hand[i].is_animating = true;
                    hand[i].target_pos = CP_Vector_Set(
                        discard_pos.x + (CARD_W_INIT * CARD_SCALE) / 2.0f,
                        discard_pos.y + (CARD_H_INIT * CARD_SCALE) / 2.0f
                    );
                }
            }
        }
    }
    // Keyboard Input
    if (current_phase == PHASE_PLAYER) {
        if (hand_size > 0) {
            if (CP_Input_KeyTriggered(KEY_A)) { selected_card_index--; if (selected_card_index < 0) selected_card_index = hand_size - 1; }
            if (CP_Input_KeyTriggered(KEY_D)) { selected_card_index++; if (selected_card_index >= hand_size) selected_card_index = 0; }
        }
        if (CP_Input_KeyTriggered(KEY_S)) { if (selected_card_index >= 0 && !hand[selected_card_index].is_discarding) card_played_this_frame = true; }
        if (CP_Input_KeyTriggered(KEY_ENTER)) {
            // End Turn
            current_phase = PHASE_ENEMY;
            enemy_action_index = 0;
            enemy_turn_timer = 0.0f;
            selected_card_index = -1;
            for (int i = 0; i < hand_size; i++) {
                hand[i].is_discarding = true;
                hand[i].is_animating = true;
                hand[i].target_pos = CP_Vector_Set(
                    discard_pos.x + (CARD_W_INIT * CARD_SCALE) / 2.0f,
                    discard_pos.y + (CARD_H_INIT * CARD_SCALE) / 2.0f
                );
            }
        }
    }

    // 14. Execute Card Effects
    if (current_phase == PHASE_PLAYER && card_played_this_frame) {
        if (selected_card_index >= 0 && !hand[selected_card_index].is_discarding) {
            Card* card = &hand[selected_card_index];
            Enemy* target_enemy = (current_enemies && selected_enemy >= 0 && selected_enemy < current_enemy_count) ? &current_enemies[selected_enemy] : NULL;
            bool has_valid_target = (target_enemy && target_enemy->alive);

            // Validation flags
            bool can_play_shield = (card->type == Shield);
            bool can_play_cleave = (card->type == Attack && card->effect == CLEAVE);
            bool can_play_heal = (card->type == Heal);
            bool can_play_attack = (card->type == Attack && card->effect != CLEAVE);

            // Execute effect only if targets are valid
            if (can_play_heal || can_play_shield || (can_play_attack && has_valid_target) || (can_play_cleave && has_valid_target)) {

                int power = hand[selected_card_index].power;
                int total_damage_dealt_this_card = 0;

                // --- ATTACK LOGIC ---
                if (card->type == Attack) {
                    int damage_to_deal = power + player.attack_bonus;
                    if (player.has_attack_boost_35) {
                        damage_to_deal = (int)(damage_to_deal * 1.35f);
                    }
                    // Special Effect: CLEAVE (AOE)
                    if (card->effect == CLEAVE) {
                        if (damage_to_deal <= 0) damage_to_deal = 1;
                        for (int i = 0; i < current_enemy_count; i++) {
                            if (current_enemies[i].alive) {
                                Enemy* cleave_target = &current_enemies[i];
                                int damage_blocked = 0;
                                int damage_dealt = 0;
                                // Shield mitigation
                                if (cleave_target->shield > 0) {
                                    damage_blocked = (damage_to_deal <= cleave_target->shield) ? damage_to_deal : cleave_target->shield;
                                    cleave_target->shield -= damage_blocked;
                                    enemy_shield_flash[i] = 0.2f;
                                }
                                damage_dealt = damage_to_deal - damage_blocked;
                                if (damage_dealt > 0) {
                                    cleave_target->health -= damage_dealt;
                                    enemy_hit_flash[i] = 0.2f;
                                    enemy_slash_timer[i] = 0.3f;
                                    total_damage_dealt_this_card += damage_dealt;

                                    // Spawn floating number
                                    float enemy_width = 120.0f;
                                    float spacing = 40.0f;
                                    float start_x = ww - (float)current_enemy_count * enemy_width - 200.0f;
                                    float enemy_x = start_x + (float)i * (enemy_width + spacing);
                                    float enemy_y = wh / 2.0f - 160.0f / 2.0f;
                                    char dmg_text[16];
                                    snprintf(dmg_text, sizeof(dmg_text), "-%d", damage_dealt);
                                    SpawnFloatingText(dmg_text, CP_Vector_Set(enemy_x + 60.0f, enemy_y - 30.0f), CP_Color_Create(255, 80, 80, 255));
                                }
                                if (cleave_target->health <= 0) cleave_target->alive = false;
                            }
                        }
                        // Lifesteal calculation (10% for AOE)
                        if (total_damage_dealt_this_card > 0) {
                            int lifesteal_amount = (int)(total_damage_dealt_this_card * 0.10f);
                            if (lifesteal_amount < 1) lifesteal_amount = 1;
                            player.health += lifesteal_amount;
                            if (player.health > player.max_health) player.health = player.max_health;
                            char heal_text[16];
                            snprintf(heal_text, sizeof(heal_text), "+%d", lifesteal_amount);
                            SpawnFloatingText(heal_text, CP_Vector_Set(175.0f, wh / 2.0f - 30.0f), CP_Color_Create(80, 255, 80, 255));
                        }
                    }
                    // Normal Single Target Attack
                    else {
                        int damage_blocked = 0;
                        int damage_dealt = 0;
                        if (target_enemy->shield > 0) {
                            damage_blocked = (damage_to_deal <= target_enemy->shield) ? damage_to_deal : target_enemy->shield;
                            target_enemy->shield -= damage_blocked;
                            enemy_shield_flash[selected_enemy] = 0.2f;
                        }
                        damage_dealt = damage_to_deal - damage_blocked;
                        if (damage_dealt > 0) {
                            target_enemy->health -= damage_dealt;
                            enemy_hit_flash[selected_enemy] = 0.2f;
                            enemy_slash_timer[selected_enemy] = 0.3f;

                            // Lifesteal calculation (50% for Single Target if Buff active)
                            if (player.has_lifesteal) {
                                int lifesteal_amount = (int)(damage_dealt * 0.50f);
                                if (lifesteal_amount < 1 && damage_dealt > 0) lifesteal_amount = 1;
                                if (lifesteal_amount > 0) {
                                    player.health += lifesteal_amount;
                                    if (player.health > player.max_health) player.health = player.max_health;
                                    CP_Sound_Play(sfx_heal);
                                    char heal_text[16];
                                    snprintf(heal_text, sizeof(heal_text), "+%d", lifesteal_amount);
                                    SpawnFloatingText(heal_text, CP_Vector_Set(175.0f, wh / 2.0f - 30.0f), CP_Color_Create(80, 255, 80, 255));
                                }
                            }

                            float enemy_width = 120.0f;
                            float spacing = 40.0f;
                            float start_x = ww - (float)current_enemy_count * enemy_width - 200.0f;
                            float enemy_x = start_x + (float)selected_enemy * (enemy_width + spacing);
                            float enemy_y = wh / 2.0f - 160.0f / 2.0f;
                            char text[16];
                            snprintf(text, sizeof(text), "-%d", damage_dealt);
                            SpawnFloatingText(text, CP_Vector_Set(enemy_x + 60.0f, enemy_y), CP_Color_Create(255, 80, 80, 255));
                        }
                        else {
                            // Blocked text
                            float enemy_width = 120.0f;
                            float spacing = 40.0f;
                            float start_x = ww - (float)current_enemy_count * enemy_width - 200.0f;
                            float enemy_x = start_x + (float)selected_enemy * (enemy_width + spacing);
                            float enemy_y = wh / 2.0f - 160.0f / 2.0f;
                            SpawnFloatingText("Block!", CP_Vector_Set(enemy_x + 60.0f, enemy_y), CP_Color_Create(150, 150, 255, 255));
                        }
                        if (target_enemy && target_enemy->health <= 0) target_enemy->alive = false;
                    }
                }
                // --- HEAL LOGIC ---
                else if (card->type == Heal) {
                    int heal_amount = power + player.heal_bonus;
                    if (player.has_heal_boost_35) {
                        heal_amount = (int)(heal_amount * 1.35f);
                    }
                    player.health += heal_amount;
                    if (player.health > player.max_health) player.health = player.max_health;

                    CP_Sound_Play(sfx_heal); // Play heal sound

                    char text[16];
                    snprintf(text, sizeof(text), "+%d", heal_amount);
                    SpawnFloatingText(text, CP_Vector_Set(175.0f, wh / 2.0f), CP_Color_Create(80, 255, 80, 255));

                    // Particle effects
                    for (int h = 0; h < 3; h++) {
                        float offsetX = (float)(h * 30 - 30);
                        float offsetY = (float)(h % 2 == 0 ? 0 : 15);
                        SpawnFloatingIcon(img_heart_particle,
                            CP_Vector_Set(175.0f + offsetX, wh / 2.0f + offsetY),
                            0.3f);
                    }

                    // Special Effect: DIVINE STRIKE (Heal damages enemies)
                    if (player.has_divine_strike || card->effect == DIVINE_STRIKE_EFFECT) {
                        int divine_damage = heal_amount / 2;
                        if (divine_damage < 1 && heal_amount > 0) divine_damage = 1;
                        if (divine_damage > 0 && current_enemies) {
                            for (int i = 0; i < current_enemy_count; i++) {
                                if (current_enemies[i].alive) {
                                    Enemy* divine_target = &current_enemies[i];
                                    int damage_blocked = 0;
                                    int damage_dealt = 0;
                                    if (divine_target->shield > 0) {
                                        damage_blocked = (divine_damage <= divine_target->shield) ? divine_damage : divine_target->shield;
                                        divine_target->shield -= damage_blocked;
                                        enemy_shield_flash[i] = 0.2f;
                                    }
                                    damage_dealt = divine_damage - damage_blocked;
                                    if (damage_dealt > 0) {
                                        divine_target->health -= damage_dealt;
                                        enemy_hit_flash[i] = 0.2f;
                                        enemy_slash_timer[i] = 0.3f;

                                        float enemy_width = 120.0f;
                                        float spacing = 40.0f;
                                        float start_x = ww - (float)current_enemy_count * enemy_width - 200.0f;
                                        float enemy_x = start_x + (float)i * (enemy_width + spacing);
                                        float enemy_y = wh / 2.0f - 160.0f / 2.0f;
                                        char dmg_text[16];
                                        snprintf(dmg_text, sizeof(dmg_text), "-%d", damage_dealt);
                                        SpawnFloatingText(dmg_text, CP_Vector_Set(enemy_x + 60.0f, enemy_y - 30.0f), CP_Color_Create(255, 255, 100, 255));
                                    }
                                    if (divine_target->health <= 0) divine_target->alive = false;
                                }
                            }
                        }
                    }
                }
                // --- SHIELD LOGIC ---
                else if (card->type == Shield) {
                    int shield_amount = power + player.shield_bonus;
                    if (player.has_shield_boost) {
                        shield_amount = (int)(shield_amount * 1.25f);
                    }
                    if (player.has_shield_boost_35) {
                        shield_amount = (int)(shield_amount * 1.35f);
                    }
                    player.shield += shield_amount;
                    player_shield_flash = 0.2f;

                    CP_Sound_Play(sfx_shield); // Play shield sound

                    char text[16];
                    snprintf(text, sizeof(text), "+%d", shield_amount);
                    SpawnFloatingText(text, CP_Vector_Set(175.0f, wh / 2.0f), CP_Color_Create(80, 80, 255, 255));

                    SpawnFloatingIcon(img_shield_particle, CP_Vector_Set(175.0f, wh / 2.0f), 0.4f);

                    // Special Effect: SHIELD BASH (Shield damages enemies)
                    if (card->effect == SHIELD_BASH) {
                        int damage_to_deal = (int)(player.shield * 0.75f);
                        if (damage_to_deal <= 0 && player.shield > 0) damage_to_deal = 1;
                        for (int i = 0; i < current_enemy_count; i++) {
                            if (current_enemies[i].alive) {
                                Enemy* bash_target = &current_enemies[i];
                                int damage_blocked = 0;
                                int damage_dealt = 0;
                                if (bash_target->shield > 0) {
                                    damage_blocked = (damage_to_deal <= bash_target->shield) ? damage_to_deal : bash_target->shield;
                                    bash_target->shield -= damage_blocked;
                                    enemy_shield_flash[i] = 0.2f;
                                }
                                damage_dealt = damage_to_deal - damage_blocked;
                                if (damage_dealt > 0) {
                                    bash_target->health -= damage_dealt;
                                    enemy_hit_flash[i] = 0.2f;
                                    enemy_slash_timer[i] = 0.3f;

                                    float enemy_width = 120.0f;
                                    float spacing = 40.0f;
                                    float start_x = ww - (float)current_enemy_count * enemy_width - 200.0f;
                                    float enemy_x = start_x + (float)i * (enemy_width + spacing);
                                    float enemy_y = wh / 2.0f - 160.0f / 2.0f;
                                    char dmg_text[16];
                                    snprintf(dmg_text, sizeof(dmg_text), "-%d", damage_dealt);
                                    SpawnFloatingText(dmg_text, CP_Vector_Set(enemy_x + 60.0f, enemy_y - 30.0f), CP_Color_Create(255, 80, 80, 255));
                                }
                                if (bash_target->health <= 0) bash_target->alive = false;
                            }
                        }
                    }
                }

                // Cleanup after using card
                played_cards++;
                card->is_discarding = true;
                card->is_animating = true;
                card->target_pos = CP_Vector_Set(
                    discard_pos.x + (CARD_W_INIT * CARD_SCALE) / 2.0f,
                    discard_pos.y + (CARD_H_INIT * CARD_SCALE) / 2.0f
                );
                // Reset Selection
                if (hand_size > 0) selected_card_index = 0;
                else selected_card_index = -1;
            }
        }
    }

    // Auto-end turn if less than 3 cards remain and 3 cards have been played
    if (current_phase == PHASE_PLAYER && GetActiveHandSize() < 3 && played_cards == 3) {
        current_phase = PHASE_ENEMY;
        enemy_action_index = 0;
        enemy_turn_timer = 0.0f;
        selected_card_index = -1;
        // discard all cards in hand on end of turn
        for (int i = 0; i < hand_size; i++) {
            hand[i].is_discarding = true;
            hand[i].is_animating = true;
            hand[i].target_pos = CP_Vector_Set(
                discard_pos.x + (CARD_W_INIT * CARD_SCALE) / 2.0f,
                discard_pos.y + (CARD_H_INIT * CARD_SCALE) / 2.0f
            );
        }
    }

    // Developer Mode Logic
    if (CP_Input_KeyTriggered(KEY_GRAVE_ACCENT)) {
        developer = developer ? 0 : 1;
    }

    // Dev Cheat: Spacebar damages selected enemy
    if (developer && CP_Input_KeyTriggered(KEY_SPACE) && current_enemies && current_enemies[selected_enemy].alive) {
        current_enemies[selected_enemy].health -= 10;
        if (current_enemies[selected_enemy].health <= 0) {
            current_enemies[selected_enemy].alive = false;
        }
        enemy_hit_flash[selected_enemy] = 0.2f;
        // Float Text logic for cheat
        float enemy_width = 120.0f;
        float spacing = 40.0f;
        float total_width = (float)current_enemy_count * enemy_width + ((float)current_enemy_count - 1.0f) * spacing;
        float start_x = ww - total_width - 200.0f;
        float enemy_x = start_x + (float)selected_enemy * (enemy_width + spacing);
        float enemy_y = wh / 2.0f - 160.0f / 2.0f;
        SpawnFloatingText("-10", CP_Vector_Set(enemy_x + 60.0f, enemy_y), CP_Color_Create(255, 255, 0, 255));
    }

    // Deck Recycling Animation
    // Automatically shuffles discard into draw if draw pile is low
    CP_Vector discard_pos_center = CP_Vector_Set(
        discard_pos.x + (CARD_W_INIT * CARD_SCALE) / 2.0f,
        discard_pos.y + (CARD_H_INIT * CARD_SCALE) / 2.0f
    );
    CP_Vector deck_pos_center = CP_Vector_Set(
        deck_pos.x + (CARD_W_INIT * CARD_SCALE) / 2.0f,
        deck_pos.y + (CARD_H_INIT * CARD_SCALE) / 2.0f
    );

    // if deck has less than the draw size and discard pile isn't 
    // recycling set all cards in discard to be recycling
    if (player_deck.size < 4 && !is_recycling) {
        for (int i = 0; i < discard_size; i++) {
            discard[i].pos = discard_pos_center;
            discard[i].target_pos = deck_pos_center;
            discard[i].is_animating = true;
        }
        is_recycling = true;
    }

    // if recycling
    if (is_recycling) {
        // use custom discard speed and animate the cards and set discard cards to deck
        for (int i = 0; i < discard_size; i++) {
            AnimateMoveCard(&discard[i], 300);
            if (!discard[i].is_animating) {
                RecycleDeck(discard, &player_deck, &discard_size);
                is_recycling = false;
            }
        }
        // draw a card to animate the recycle
        if (discard_size > 0) {
            DrawCard(&discard[0]);
        }
    }
}

// Cleans up assets when game state exits.
void Game_Exit(void)
{
    CP_Image_Free(game_bg);
    CP_Sound_Free(background_music);
    CP_Sound_Free(sfx_shield);
    CP_Sound_Free(sfx_heal);
    CP_Sound_Free(sfx_draw);
}