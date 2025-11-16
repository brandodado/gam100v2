#include <math.h>
#include <string.h> // For strcmp
#include <stdlib.h> // For rand()
#include "cprocessing.h"
#include "mainmenu.h"
#include "utils.h"
#include <stdbool.h>
#include "card.h"
#include "levels.h"
#include <stdio.h>
#include "game.h"
#include "shop.h" // Include the shop state

// Turn state enum
typedef enum {
    PLAYER_TURN,
    ENEMY_TURN,
    GAME_OVER
} GameState;


// init window size for circle position settings
int window_w;
int window_h;

int selected_card_index;


#define SELECT_BTN_W 200
#define SELECT_BTN_H 100

// Button defines for GAME_OVER screen
#define CHOICE_BTN_W 400.0f
#define CHOICE_BTN_H 100.0f

CP_Vector select_btn_pos;

CP_Vector deck_pos;
CP_Vector discard_pos;

int played_cards;

int turn_num;
bool drawn;

int animate_index;


float max_bar_width;

#define MAX_HAND_SIZE 7	
#define MAX_DECK_SIZE 25

Card hand[MAX_HAND_SIZE];
Card deck[MAX_DECK_SIZE];
int hand_size;
int deck_size;

Card discard[MAX_DECK_SIZE];
int discard_size;

// ---------------- Globals ----------------

static Player player; // Set in Game_Init

static int selected_enemy = 0;
static float hit_flash_timer[16] = { 0.0f };
static float revive_text_timer[16] = { 0.0f };
static float thief_steal_timer[16] = { 0.0f };
static float dot_text_timer[16] = { 0.0f };

static float player_miss_timer = 0.0f;
static float enemy_miss_timer[16] = { 0.0f };

static Enemy* current_enemies = NULL;
static int current_enemy_count = 0;

// --- UPDATED: Removed 'static' ---
int current_level = 1;

static bool stage_cleared = false;
static float stage_clear_timer = 0.0f;

// --- UPDATED: Removed 'static' ---
CP_Font game_font = 0;

// --- Icon Globals ---
static CP_Image card_type_icons[3];
static CP_Image card_effect_icons[5];


static GameState current_turn;

// Definition of the global card multiplier
float g_card_multiplier;

// Definition of the global lifesteal flag
bool g_player_has_lifesteal;

// Definition of the global card draw flag
bool g_player_draw_bonus;

// Definition of the checkpoint flag
bool g_player_has_died = false;

// Globals for Enemy Animation
static float enemy_turn_timer = 0.0f;
static int enemy_action_index = 0;
static float enemy_anim_offset_x = 0.0f;
static bool enemy_has_hit = false;


// ---------------- Helpers ----------------

// Helper function for miss chance
static bool DidAttackMiss(void)
{
    return (rand() % 100) < 35;
}

// --- UPDATED: Removed 'static' ---
void LoadLevel(int level) {
    Enemy* new_enemies = NULL;
    int new_enemy_count = 0;

    if (level == 1) {
        new_enemies = level1_enemies;
        new_enemy_count = level1_enemy_count;
    }
    else if (level == 2) {
        new_enemies = level2_enemies;
        new_enemy_count = level2_enemy_count;
    }
    else if (level == 3) {
        new_enemies = level3_enemies;
        new_enemy_count = level3_enemy_count;
    }
    else {
        // No more levels → back to menu
        CP_Engine_SetNextGameState(Main_Menu_Init, Main_Menu_Update, Main_Menu_Exit);
        return; // <-- Fix for NULL pointer
    }

    current_enemies = new_enemies;
    current_enemy_count = new_enemy_count;

    // Reset per-level state
    selected_enemy = 0;
    player_miss_timer = 0.0f;
    for (int i = 0; i < 16; i++) {
        hit_flash_timer[i] = 0.0f;
        revive_text_timer[i] = 0.0f;
        thief_steal_timer[i] = 0.0f;
        enemy_miss_timer[i] = 0.0f;
        dot_text_timer[i] = 0.0f;
    }
    stage_cleared = false;
    stage_clear_timer = 0.0f;

    // Reset animation state
    enemy_turn_timer = 0.0f;
    enemy_action_index = 0;
    enemy_anim_offset_x = 0.0f;
    enemy_has_hit = false;
}

// Helper function to draw buttons on the game over screen
static void DrawChoiceButton(const char* text, float x, float y, float w, float h, int is_hovered)
{
    CP_Settings_RectMode(CP_POSITION_CENTER);

    // Draw button rectangle
    if (is_hovered) {
        CP_Settings_Fill(CP_Color_Create(150, 150, 150, 255)); // Brighter
    }
    else {
        CP_Settings_Fill(CP_Color_Create(80, 80, 80, 255)); // Dark gray
    }

    CP_Settings_StrokeWeight(2.0f);
    CP_Settings_Stroke(CP_Color_Create(200, 200, 200, 255));
    CP_Graphics_DrawRect(x, y, w, h);

    // Draw button text
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Font_Set(game_font);
    CP_Settings_TextSize(24.0f);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Font_DrawText(text, x, y + 2.0f);
}

// --- UPDATED: Removed 'static' ---
void ExecuteLevelUpChoice(int health_bonus, float multiplier_increase, bool enable_lifesteal, bool enable_draw_bonus)
{
    // Apply the chosen stats
    g_card_multiplier += multiplier_increase;
    player.max_health += health_bonus;

    if (enable_lifesteal) {
        g_player_has_lifesteal = true;
    }
    if (enable_draw_bonus) {
        g_player_draw_bonus = true;
    }
}

// --- UPDATED: Removed 'static' ---
void Game_ResetDeathFlag(void)
{
    g_player_has_died = false;
}

static bool AllEnemiesDefeated(void) {
    for (int i = 0; i < current_enemy_count; i++) {
        if (current_enemies[i].alive && current_enemies[i].health > 0) {
            return false;
        }
    }
    return true;
}

static void HandleInput(void) {
    // Targeting controls
    if (CP_Input_KeyTriggered(KEY_LEFT)) {
        selected_enemy--;
        if (selected_enemy < 0) selected_enemy = current_enemy_count - 1;
    }
    if (CP_Input_KeyTriggered(KEY_RIGHT)) {
        selected_enemy++;
        if (selected_enemy >= current_enemy_count) selected_enemy = 0;
    }

    // Attack selected enemy
    if (CP_Input_KeyTriggered(KEY_SPACE)) {
        if (current_enemy_count > 0) {
            Enemy* e = &current_enemies[selected_enemy];
            if (e->alive) {

                if (DidAttackMiss()) {
                    enemy_miss_timer[selected_enemy] = 1.5f; // Show "MISS!"
                }
                else {
                    e->health -= player.attack;
                    if (e->health <= 0) {
                        e->health = 0;
                        e->alive = false;
                    }
                    if (e->alive && selected_enemy >= 0 && selected_enemy < 16)
                        hit_flash_timer[selected_enemy] = 0.2f;
                }
            }
        }
    }
}

static void UpdateStageClear(void) {
    // Trigger stage clear once, then run timer and advance
    if (!stage_cleared && AllEnemiesDefeated() && current_enemy_count > 0) {
        stage_cleared = true;
        stage_clear_timer = 2.0f; // seconds
    }

    if (stage_cleared) {
        stage_clear_timer -= CP_System_GetDt();
        if (stage_clear_timer <= 0.0f) {
            stage_cleared = false;

            // Set the state to the new Shop
            CP_Engine_SetNextGameState(Shop_Init, Shop_Update, Shop_Exit);
        }
    }
}

static void DrawEntity(const char* name, int health, int max_health,
    int attack, int defense,
    float x, float y, float w, float h,
    CP_Color model_color,
    int is_selected)
{
    // Always use corner mode for entity drawing
    CP_Settings_RectMode(CP_POSITION_CORNER);

    // Glow outline if selected
    if (is_selected) {
        CP_Settings_StrokeWeight(4.0f);
        CP_Settings_Stroke(CP_Color_Create(255, 255, 0, 255)); // yellow glow
        CP_Graphics_DrawRect(x - 4.0f, y - 4.0f, w + 8.0f, h + 8.0f);
    }
    // Reset stroke for normal draws
    CP_Settings_StrokeWeight(0);

    // Logic for Hit Flash
    if (hit_flash_timer[selected_enemy] > 0.0f && is_selected) {
        model_color = CP_Color_Create(255, 0, 0, 255); // Flash Red
        hit_flash_timer[selected_enemy] -= CP_System_GetDt();
    }

    // Clamp health for bar fill (but show actual values in text)
    int clamped_health = health;
    if (clamped_health < 0) clamped_health = 0;
    if (clamped_health > max_health) clamped_health = max_health;
    float ratio = (max_health > 0) ? (float)clamped_health / (float)max_health : 0.0f;

    // Entity model
    CP_Settings_Fill(model_color);
    CP_Graphics_DrawRect(x, y, w, h);

    // Name directly above entity, centered
    CP_Font_Set(game_font);
    CP_Settings_TextSize(20);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Font_DrawText(name, x + w * 0.5f, y - 20.0f);

    // HP bar snug below entity
    float bar_h = 16.0f;
    float bar_y = y + h + 4.0f;

    // Bar background
    CP_Settings_Fill(CP_Color_Create(70, 70, 70, 255));
    CP_Graphics_DrawRect(x, bar_y, w, bar_h);

    // Bar fill
    CP_Color hp_color = (ratio > 0.6f) ? CP_Color_Create(0, 200, 0, 255)
        : (ratio > 0.3f) ? CP_Color_Create(255, 200, 0, 255)
        : CP_Color_Create(200, 0, 0, 255);
    CP_Settings_Fill(hp_color);
    CP_Graphics_DrawRect(x, bar_y, w * ratio, bar_h);

    // HP text centered inside the bar
    char hp_text[32];
    snprintf(hp_text, sizeof(hp_text), "%d/%d", health, max_health);
    CP_Settings_TextSize(16);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Font_DrawText(hp_text, x + w * 0.5f, bar_y + bar_h * 0.5f + 2.0f);

    // Stats directly below HP bar
    CP_Settings_TextSize(18);

    // ATK
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Settings_Fill(CP_Color_Create(255, 80, 80, 255));
    char atk_text[32]; snprintf(atk_text, sizeof(atk_text), "ATK: %d", attack);
    CP_Font_DrawText(atk_text, x + 6.0f, bar_y + bar_h + 18.0f);

    // DEF
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Settings_Fill(CP_Color_Create(80, 120, 255, 255));
    char def_text[32]; snprintf(def_text, sizeof(def_text), "DEF: %d", defense);
    CP_Font_DrawText(def_text, x + w * 0.5f, bar_y + bar_h + 18.0f);
}

static void DrawPlayer(Player* p, float ww, float wh) {
    DrawEntity("Player", player.health, player.max_health,
        player.attack, player.defense,
        100.0f, wh / 2.0f - 100.0f, 150.0f, 200.0f,
        CP_Color_Create(50, 50, 150, 255),
        selected_enemy == -1); // highlight player if no enemy selected
}

static void DrawEnemy(Enemy* e, int index,
    float x, float y, float w, float h) {
    DrawEntity(e->name, e->health, e->max_health,
        e->attack, e->defense,
        x, y, w, h,
        CP_Color_Create(120, 120, 120, 255),
        selected_enemy == index); // highlight if this enemy is selected
}

static void DrawHUD(float ww, float wh) {
    // Debug overlay
    char debug[128];
    snprintf(debug, sizeof(debug), "Level %d | Enemies %d", current_level, current_enemy_count);
    CP_Settings_Fill(CP_Color_Create(255, 255, 0, 255));
    CP_Font_Set(game_font);
    CP_Settings_TextSize(20);
    CP_Font_DrawText(debug, 20.0f, 35.0f);

    // Stage clear banner
    if (stage_cleared) {
        CP_Settings_Fill(CP_Color_Create(255, 255, 0, 255));
        CP_Settings_TextSize(36);
        CP_Font_DrawText("STAGE CLEARED!", ww / 2.0f - 120.0f, wh / 2.0f);
    }
}

static void UpdateEnemyTurn(void)
{
    float dt = CP_System_GetDt();
    enemy_turn_timer += dt;

    // --- 1. Check if all enemies have acted ---
    if (enemy_action_index >= current_enemy_count) {
        current_turn = PLAYER_TURN;
        turn_num++;
        played_cards = 0;
        drawn = false;
        return; // This enemy turn is over
    }

    // --- 2. Get the current acting enemy ---
    Enemy* e = &current_enemies[enemy_action_index];

    // --- 3. Skip dead enemies ---
    if (!e->alive) {
        enemy_action_index++;
        enemy_turn_timer = 0.0f;
        return; // Go to the next enemy
    }

    // --- 4. Lunge Forward (Animation) ---
    if (enemy_turn_timer < 0.3f) {
        float ratio = enemy_turn_timer / 0.3f;
        enemy_anim_offset_x = -200.0f * ratio; // Move left
    }
    // --- 5. Hit Frame (Apply AI Logic) ---
    else if (!enemy_has_hit)
    {
        // --- Apply DOT Damage ---
        if (e->dot_timing > 0) {
            int dot_damage = 5; // Base DOT damage
            e->health -= dot_damage;
            dot_text_timer[enemy_action_index] = 1.0f; // Show "DOT!" text
            e->dot_timing--; // Decrement DOT timer
            if (e->health <= 0) {
                e->alive = false;
            }
        }

        // Only perform main action if still alive after DOT
        if (e->alive) {
            // --- WITCH AI ---
            if (strcmp(e->name, "Witch") == 0)
            {
                // Try to find a dead ally
                int dead_ally_index = -1;
                for (int j = 0; j < current_enemy_count; j++) {
                    if (!current_enemies[j].alive) {
                        dead_ally_index = j;
                        break;
                    }
                }

                if (dead_ally_index != -1)
                {
                    // --- Revive the ally ---
                    current_enemies[dead_ally_index].alive = true;
                    current_enemies[dead_ally_index].health = current_enemies[dead_ally_index].max_health / 2;
                    revive_text_timer[dead_ally_index] = 1.5f;
                }
                else {
                    // If no one to revive, just attack
                    if (DidAttackMiss()) {
                        player_miss_timer = 1.5f; // Player dodged
                    }
                    else {
                        int damage = e->attack - player.defense;
                        if (damage < 1) damage = 1;
                        player.health -= damage;
                    }
                }
            }
            // --- GOBLIN THIEF AI ---
            else if (strcmp(e->name, "Goblin Thief") == 0)
            {
                // Special Ability: Discard from Hand
                if (hand_size > 0) {
                    int card_to_steal_index = 0; // Steal the first card
                    DiscardCard(hand, &card_to_steal_index, &hand_size, discard, &discard_size);
                    SetHandPos(hand, hand_size); // Re-center the hand
                    thief_steal_timer[enemy_action_index] = 1.5f; // Trigger the UI
                }

                // Standard Attack
                if (DidAttackMiss()) {
                    player_miss_timer = 1.5f; // Player dodged
                }
                else {
                    int damage = e->attack - player.defense;
                    if (damage < 1) damage = 1;
                    player.health -= damage;
                }
            }
            // --- ORC/SLIME (Standard Attack) ---
            else
            {
                if (DidAttackMiss()) {
                    player_miss_timer = 1.5f; // Player dodged
                }
                else {
                    int damage = e->attack - player.defense;
                    if (damage < 1) damage = 1;
                    player.health -= damage;
                }
            }
        }

        // --- End of AI Logic ---
        enemy_has_hit = true;
        enemy_anim_offset_x = -200.0f; // Hold at max lunge
    }
    // --- 6. Lunge Back (Animation) ---
    else if (enemy_turn_timer < 0.6f) {
        float ratio = (enemy_turn_timer - 0.3f) / 0.3f; // 0.0 to 1.0
        enemy_anim_offset_x = -200.0f * (1.0f - ratio); // Move right
    }
    // --- 7. Action Finished (Move to next enemy) ---
    else {
        enemy_action_index++;       // Move to next enemy
        enemy_turn_timer = 0.0f;    // Reset timer
        enemy_anim_offset_x = 0.0f; // Reset position
        enemy_has_hit = false;    // Reset hit flag
    }

    // --- 8. CHECK FOR PLAYER DEATH (Immediate) ---
    if (player.health <= 0)
    {
        current_turn = GAME_OVER;
        return;
    }
}

// --- NEW: Helper to load card icons ---
static void LoadCardIcons(void)
{
    // These paths must be correct in your project
    card_type_icons[Attack] = CP_Image_Load("Assets/sword.png");
    card_type_icons[Heal] = CP_Image_Load("Assets/suit_hearts.png");
    // card_type_icons[Shield] = CP_Image_Load("Assets/shield.png"); // If you add shield back

    card_effect_icons[None] = NULL;
    card_effect_icons[DOT] = CP_Image_Load("Assets/fire.png");
    card_effect_icons[Draw] = CP_Image_Load("Assets/card_add.png");
}

void Game_Init(void)
{
    // --- Reset all player stats ---
    player.health = 80;
    player.max_health = 80;
    player.attack = 7;
    player.defense = 0;
    g_card_multiplier = 1.0f;
    g_player_has_lifesteal = false;
    g_player_draw_bonus = false;
    g_player_has_died = false;

    current_level = 1;
    LoadLevel(current_level);

    // --- Load font ONCE ---
    if (game_font == 0) {
        game_font = CP_Font_Load("Assets/Exo2-Regular.ttf");
        if (game_font == 0) {
            printf("Font failed to load!\n");
        }
    }
    CP_Font_Set(game_font); // Always set it as active
    CP_Settings_TextSize(24);

    // --- NEW: Load card icons ---
    LoadCardIcons();

    if (current_enemies == NULL || current_enemy_count <= 0) {
        static Enemy fallback[] = { { "Fallback", 10, 10, 1, 0, true, 0 } }; // Added 0 for dot_timing
        current_enemies = fallback;
        current_enemy_count = 1;
    }

    deck_pos = CP_Vector_Set(50, 600);
    discard_pos = CP_Vector_Set(200, 600);

    // --- UPDATED: New card definitions ---
    Card strike = { deck_pos, CP_Vector_Set(0, 0), Attack, None, 10.0f,
                    "Strike", "Deal %d damage",
                    CARD_W_INIT, CARD_H_INIT, false };
    Card heal = { deck_pos, CP_Vector_Set(0, 0), Heal, None, 15.0f,
                  "Heal", "Heal %d health",
                  CARD_W_INIT, CARD_H_INIT, false };
    Card hemorrhage = { deck_pos, CP_Vector_Set(0, 0), Attack, DOT, 5.0f,
                  "Hemorrhage", "Deal %d damage.\nApply 3 DOT",
                  CARD_W_INIT, CARD_H_INIT, false };
    Card quick_think = { deck_pos, CP_Vector_Set(0, 0), Heal, Draw, 5.0f,
                  "Quick Think", "Heal %d.\nDraw 1 card",
                  CARD_W_INIT, CARD_H_INIT, false };
    // --- END UPDATED ---

    // --- UPDATED: New starting deck ---
    deck[0] = strike;
    deck[1] = strike;
    deck[2] = strike;
    deck[3] = strike;
    deck[4] = strike;
    deck[5] = heal;
    deck[6] = heal;
    deck[7] = hemorrhage;
    deck[8] = hemorrhage;
    deck[9] = quick_think;
    // --- END UPDATED ---

    deck_size = 10;
    hand_size = 0;
    discard_size = 0;
    selected_card_index = -1;
    played_cards = 0;

    turn_num = 0;
    drawn = 0;

    current_turn = PLAYER_TURN;

    // --- Init animation globals ---
    enemy_turn_timer = 0.0f;
    enemy_action_index = 0;
    enemy_anim_offset_x = 0.0f;
    enemy_has_hit = false;
}

void Game_Init_At_Level_2_Shop(void)
{
    // 1. Run the normal init to reset stats, deck, etc.
    Game_Init();
    // 2. Now, override the state to put the player at the Level 2 shop
    current_level = 2; // We just "cleared" level 2

    // We must also set the game font here
    CP_Font_Set(game_font);

    // Set the state to the new Shop
    CP_Engine_SetNextGameState(Shop_Init, Shop_Update, Shop_Exit);
}


void Game_Update(void) {
    CP_Graphics_ClearBackground(CP_Color_Create(20, 25, 28, 255));

    float ww = (float)CP_System_GetWindowWidth();
    float wh = (float)CP_System_GetWindowHeight();

    // --- Global Input ---
    if (CP_Input_KeyTriggered(KEY_Q)) {
        CP_Engine_SetNextGameState(Main_Menu_Init, Main_Menu_Update, Main_Menu_Exit);
    }

    // Global Logic
    UpdateStageClear();

    // --- Player (left side) ---
    CP_Settings_RectMode(CP_POSITION_CORNER);
    DrawEntity("Player", player.health, player.max_health,
        player.attack, player.defense,
        100.0f, wh / 2.0f - 100.0f, 150.0f, 200.0f,
        CP_Color_Create(50, 50, 150, 255),
        selected_enemy == -1); // glow player if no enemy selected

    // --- Draw Player MISS Text ---
    if (player_miss_timer > 0.0f) {
        CP_Font_Set(game_font);
        CP_Settings_TextSize(24);
        CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
        CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255)); // White text
        // Draw over player model
        CP_Font_DrawText("MISS!", 100.0f + 75.0f, wh / 2.0f - 100.0f + 100.0f);
        player_miss_timer -= CP_System_GetDt();
    }

    // --- Enemies (right side) ---
    CP_Settings_RectMode(CP_POSITION_CORNER);
    if (current_enemies && current_enemy_count > 0) {
        float enemy_width = 120.0f;
        float enemy_height = 160.0f;
        float spacing = 40.0f;
        float total_width = current_enemy_count * enemy_width + (current_enemy_count - 1) * spacing;
        float start_x = ww - total_width - 200.0f;
        float y = wh / 2.0f - enemy_height / 2.0f;

        for (int i = 0; i < current_enemy_count; i++) {
            Enemy* e = &current_enemies[i];
            float x = start_x + i * (enemy_width + spacing);

            // --- Apply animation offset ---
            if (current_turn == ENEMY_TURN && i == enemy_action_index) {
                x += enemy_anim_offset_x;
            }

            DrawEntity(e->name, e->health, e->max_health,
                e->attack, e->defense,
                x, y, enemy_width, enemy_height,
                CP_Color_Create(120, 120, 120, 255),
                selected_enemy == i); // glow if this enemy is selected
        }

        // --- Draw Revive Indicators ---
        for (int i = 0; i < current_enemy_count; i++) {
            if (revive_text_timer[i] > 0.0f) {
                float x = start_x + i * (enemy_width + spacing);
                CP_Font_Set(game_font);
                CP_Settings_TextSize(24);
                CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
                CP_Settings_Fill(CP_Color_Create(0, 255, 0, 255)); // Bright Green text
                CP_Font_DrawText("REVIVED!", x + enemy_width / 2.0f, y - 40.0f);
                revive_text_timer[i] -= CP_System_GetDt();
            }
        }

        // --- Draw Thief Steal Indicators ---
        for (int i = 0; i < current_enemy_count; i++) {
            if (thief_steal_timer[i] > 0.0f) {
                float x = start_x + i * (enemy_width + spacing);
                CP_Font_Set(game_font);
                CP_Settings_TextSize(24);
                CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
                CP_Settings_Fill(CP_Color_Create(255, 100, 100, 255)); // Red text
                CP_Font_DrawText("CARD STOLEN!", x + enemy_width / 2.0f, y - 65.0f); // Draw slightly higher
                thief_steal_timer[i] -= CP_System_GetDt();
            }
        }

        // --- Draw Enemy MISS Text ---
        for (int i = 0; i < current_enemy_count; i++) {
            if (enemy_miss_timer[i] > 0.0f) {
                float x = start_x + i * (enemy_width + spacing);
                CP_Font_Set(game_font);
                CP_Settings_TextSize(24);
                CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
                CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255)); // White text
                // Draw over enemy model
                CP_Font_DrawText("MISS!", x + enemy_width / 2.0f, y + enemy_height / 2.0f);
                enemy_miss_timer[i] -= CP_System_GetDt();
            }
        }

        // --- NEW: Draw DOT Text ---
        for (int i = 0; i < current_enemy_count; i++) {
            if (dot_text_timer[i] > 0.0f) {
                float x = start_x + i * (enemy_width + spacing);
                CP_Font_Set(game_font);
                CP_Settings_TextSize(24);
                CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
                CP_Settings_Fill(CP_Color_Create(160, 32, 240, 255)); // Purple text
                CP_Font_DrawText("DOT: 5 DMG!", x + enemy_width / 2.0f, y + enemy_height / 2.0f);
                dot_text_timer[i] -= CP_System_GetDt();
            }
        }
    }

    // --- Global HUD overlays ---
    DrawHUD(ww, wh);

    // --- TURN-BASED LOGIC ---
    switch (current_turn)
    {
    case PLAYER_TURN:
    {
        // --- PLAYER BUFF INDICATOR UI ---
        float buff_text_y = 60.0f; // Start below the debug text
        CP_Font_Set(game_font);
        CP_Settings_TextSize(18);
        CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);

        if (g_player_has_lifesteal) {
            CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
            CP_Font_DrawText("Vampire (Lifesteal)", 20.0f, buff_text_y);
            buff_text_y += 20.0f; // Move down for next buff
        }
        if (g_card_multiplier > 1.0f) {
            char multiplier_text[64];
            snprintf(multiplier_text, 64, "Focused (Cards x%.1f)", g_card_multiplier);
            CP_Settings_Fill(CP_Color_Create(255, 255, 0, 255));
            CP_Font_DrawText(multiplier_text, 20.0f, buff_text_y);
            buff_text_y += 20.0f;
        }
        if (g_player_draw_bonus) {
            CP_Settings_Fill(CP_Color_Create(0, 150, 255, 255));
            CP_Font_DrawText("Reserves (+1 Card Draw)", 20.0f, buff_text_y);
        }
        // --- END NEW UI ---


        // Player-only input
        HandleInput();

        // --- Card system ---
        CP_Settings_Stroke(CP_Color_Create(0, 0, 0, 255));
        CP_Settings_Fill(CP_Color_Create(145, 145, 145, 255));
        CP_Graphics_DrawRect(deck_pos.x, deck_pos.y, CARD_W_INIT, CARD_H_INIT);
        CP_Graphics_DrawRect(discard_pos.x, discard_pos.y, CARD_W_INIT, CARD_H_INIT);

        int cards_to_draw = 3;
        if (g_player_draw_bonus) {
            cards_to_draw = 4;
        }

        // Initial draw
        if (turn_num == 0 && !drawn) {
            for (int i = 0; i < cards_to_draw; ++i) { // Use new variable
                if (deck_size > 0)
                    DealFromDeck(deck, &hand[hand_size], &deck_size, &hand_size);
            }
            SetHandPos(hand, hand_size);
        }
        else if (!drawn) {
            // Regular draw
            if (deck_size > 0) {
                DealFromDeck(deck, &hand[hand_size], &deck_size, &hand_size);
            }
            // Check if we need to draw the bonus card
            if (g_player_draw_bonus && deck_size > 0) {
                if (deck_size > 0) // Check again in case deck ran out
                    DealFromDeck(deck, &hand[hand_size], &deck_size, &hand_size);
            }
            SetHandPos(hand, hand_size);
        }
        drawn = true;

        // Mulligan system
        if (deck_size == 0 && hand_size == 0 && played_cards == 0) { // Only recycle if hand is empty
            RecycleDeck(discard, deck, &discard_size, &deck_size);
        }

        // Animate card movement
        if (hand_size > 0) {
            if (animate_index >= hand_size) animate_index = 0; // Fix potential crash
            if (hand[animate_index].is_animating) {
                AnimateMoveCard(&hand[animate_index]);
            }
            else {
                animate_index++;
                if (animate_index >= hand_size) animate_index = 0;
            }
        }

        // Draw all cards in hand
        for (int i = 0; i < hand_size; ++i) {
            if (i == selected_card_index) {
                CP_Settings_Stroke(CP_Color_Create(255, 255, 0, 255));
            }
            else {
                CP_Settings_Stroke(CP_Color_Create(0, 0, 0, 255));
            }
            // --- UPDATED: Pass icons to DrawCard ---
            DrawCard(&hand[i], card_type_icons, card_effect_icons);
        }

        // Select button
        float select_btn_x = ww - 150.0f;
        float select_btn_y = wh - 100.0f;
        if (selected_card_index >= 0 || played_cards > 0) {
            CP_Settings_RectMode(CP_POSITION_CENTER);
            CP_Settings_Fill(CP_Color_Create(100, 100, 100, 255));
            CP_Graphics_DrawRect(select_btn_x, select_btn_y, SELECT_BTN_W, SELECT_BTN_H);

            // --- Draw Button Text ---
            CP_Font_Set(game_font);
            CP_Settings_TextSize(24);
            CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
            CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));

            if (selected_card_index >= 0) {
                CP_Font_DrawText("Use Card", select_btn_x, select_btn_y + 2.0f);
            }
            else {
                CP_Font_DrawText("End Turn", select_btn_x, select_btn_y + 2.0f);
            }
        }

        // Player card selection
        for (int i = 0; i < hand_size; ++i) {
            if (CP_Input_MouseClicked() &&
                IsAreaClicked(hand[i].pos.x, hand[i].pos.y, hand[i].card_w, hand[i].card_h,
                    CP_Input_GetMouseX(), CP_Input_GetMouseY()) == 1) {
                SelectCard(i, &selected_card_index);
            }
        }

        // Use card
        if (CP_Input_MouseClicked() &&
            IsAreaClicked(select_btn_x, select_btn_y, SELECT_BTN_W, SELECT_BTN_H,
                CP_Input_GetMouseX(), CP_Input_GetMouseY()) == 1 &&
            selected_card_index >= 0) {

            Enemy* targeted_enemy = &current_enemies[selected_enemy];
            bool did_miss = false;

            // Check for miss *before* using the card
            if (hand[selected_card_index].type == Attack) {
                if (DidAttackMiss()) {
                    enemy_miss_timer[selected_enemy] = 1.5f; // Show "MISS!"
                    did_miss = true;
                }
            }

            // --- UPDATED: Pass deck and deck_size to UseCard ---
            UseCard(hand, &selected_card_index, &hand_size, &player, targeted_enemy, deck, &deck_size, did_miss);

            if (!did_miss) {
                if (targeted_enemy->health <= 0) {
                    targeted_enemy->health = 0;
                    targeted_enemy->alive = false;
                }

                if (targeted_enemy->alive) {
                    hit_flash_timer[selected_enemy] = 0.2f;
                }
            }

            // This logic runs whether it was a hit or miss
            ++played_cards;

            // selected_index is reset inside UseCard if it was a Draw card
            // so we must check again
            if (selected_card_index != -1 && hand_size > 0) {
                hand[selected_card_index].target_pos = discard_pos;
            }
        }
        else if (CP_Input_MouseClicked() &&
            IsAreaClicked(select_btn_x, select_btn_y, SELECT_BTN_W, SELECT_BTN_H,
                CP_Input_GetMouseX(), CP_Input_GetMouseY()) == 1 &&
            played_cards > 0) {
            played_cards = 3;
            SetHandPos(hand, hand_size);
            animate_index = 0;
        }

        // Move selected card to discard
        if (selected_card_index >= 0 && hand_size > 0 &&
            hand[selected_card_index].pos.x == discard_pos.x &&
            hand[selected_card_index].pos.y == discard_pos.y) {
            DiscardCard(hand, &selected_card_index, &hand_size, discard, &discard_size);
        }

        // End turn
        if (played_cards == 3) {
            current_turn = ENEMY_TURN;
            // Reset animation vars
            enemy_action_index = 0;
            enemy_turn_timer = 0.0f;
            enemy_anim_offset_x = 0.0f;
            enemy_has_hit = false;
        }
        break; // End PLAYER_TURN
    }

    case ENEMY_TURN:
    {
        UpdateEnemyTurn();
        break; // End ENEMY_TURN
    }

    case GAME_OVER:
    {
        // --- 1. Draw the UI ---
        CP_Settings_Fill(CP_Color_Create(0, 0, 0, 200));
        CP_Settings_RectMode(CP_POSITION_CORNER);
        CP_Graphics_DrawRect(0, 0, ww, wh);

        CP_Font_Set(game_font);
        CP_Settings_TextSize(48);
        CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
        CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255)); // Red text
        CP_Font_DrawText("YOU HAVE DIED", ww / 2.0f, wh / 2.0f - 150.0f);

        // Set the death flag
        g_player_has_died = true;

        // Define button positions
        float buttonA_x = ww / 2.0f;
        float buttonA_y = wh / 2.0f;
        float buttonB_x = ww / 2.0f;
        float buttonB_y = wh / 2.0f + 120.0f;

        float mouse_x = (float)CP_Input_GetMouseX();
        float mouse_y = (float)CP_Input_GetMouseY();
        int mouse_clicked = CP_Input_MouseClicked();

        int hoverA = IsAreaClicked(buttonA_x, buttonA_y, CHOICE_BTN_W, CHOICE_BTN_H, mouse_x, mouse_y);
        int hoverB = IsAreaClicked(buttonB_x, buttonB_y, CHOICE_BTN_W, CHOICE_BTN_H, mouse_x, mouse_y);

        DrawChoiceButton("Restart", buttonA_x, buttonA_y, CHOICE_BTN_W, CHOICE_BTN_H, hoverA);
        DrawChoiceButton("Main Menu", buttonB_x, buttonB_y, CHOICE_BTN_W, CHOICE_BTN_H, hoverB);

        // --- 2. Check for Click ---
        if (mouse_clicked) {
            if (hoverA) {
                // "Restart" was clicked.
                CP_Engine_SetNextGameState(Game_Init, Game_Update, Game_Exit);
            }
            else if (hoverB) {
                // "Main Menu" was clicked
                CP_Engine_SetNextGameState(Main_Menu_Init, Main_Menu_Update, Main_Menu_Exit);
            }
        }
        break; // End GAME_OVER
    }
    }

}

void Game_Exit(void)
{
    // This function is called when changing states.
    // We MUST NOT free the font here.
}