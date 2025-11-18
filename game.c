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
#include "deck.h" 
#include "gameover.h" 
#include "buff_reward.h" 
#include "victory.h" 

// ---------------------------------------------------------
// 1. GLOBAL VARIABLES
// ---------------------------------------------------------

int selected_card_index;

#define SELECT_BTN_W 200
#define SELECT_BTN_H 100
#define MAX_LEVEL 9 

CP_Vector deck_pos;
CP_Vector discard_pos;

int played_cards;
int turn_num;
bool drawn;

#define MAX_HAND_SIZE 7      
Card hand[MAX_HAND_SIZE];
Card draw_pile[MAX_DECK_SIZE];
int draw_pile_size;
Card discard[MAX_DECK_SIZE];
int discard_size;
int hand_size;

Deck player_deck;
RewardState reward_state;
BuffRewardState buff_reward_state;

static bool g_is_restarting_from_checkpoint = false;

// --- Player/Game State ---
#define START_ATTACK 7
#define START_SHIELD 0 
#define START_HEALTH 80
#define INITIAL_DECK_SIZE 14 // (6 atk + 4 heal + 4 shield)

static Player player = {
    START_HEALTH, START_HEALTH, START_ATTACK, START_SHIELD,
    false, false, false, false, // lifesteal, desperate_draw, divine_strike, shield_boost
    false, false, false,        // attack_boost_35, heal_boost_35, shield_boost_35
    1,                          // checkpoint_level
    0,                          // death_count
    0, 0, 0, 0                  // bonus trackers
};
static int selected_enemy = 0;

static Enemy* current_enemies = NULL;
static int current_enemy_count = 0;
static int current_level = 1;

static bool stage_cleared = false;
static float banner_timer = 0.0f;
static bool reward_active = false;
static bool buff_reward_active = false;

static float player_hit_flash = 0.0f;
static float player_shield_flash = 0.0f;
static float enemy_hit_flash[16] = { 0.0f };
static float enemy_shield_flash[16] = { 0.0f };

static FloatingText floating_texts[MAX_FLOATING_TEXTS];
static int floating_text_count = 0;

static CP_Font game_font;
static CP_Image game_bg = NULL;

typedef enum {
    PHASE_PLAYER,
    PHASE_ENEMY
} BattlePhase;

BattlePhase current_phase = PHASE_PLAYER;
float enemy_turn_timer = 0.0f;
int enemy_action_index = 0;
float enemy_anim_offset_x = 0.0f;
bool enemy_has_hit = false;


// ---------------------------------------------------------
// 2. HELPER FUNCTIONS
// ---------------------------------------------------------

void Game_Set_Restart_Flag(bool value) {
    g_is_restarting_from_checkpoint = value;
}

void Game_Increment_Death_Count(void) {
    player.death_count++;
}

int Game_Get_Death_Count(void) {
    return player.death_count;
}

static void SpawnFloatingText(const char* text, CP_Vector pos, CP_Color color) {
    if (floating_text_count >= MAX_FLOATING_TEXTS) return;

    FloatingText* ft = &floating_texts[floating_text_count];
    snprintf(ft->text, sizeof(ft->text), "%s", text);
    ft->pos = pos;
    ft->color = color;
    ft->timer = 1.0f; // 1 second lifetime
    floating_text_count++;
}
static void UpdateAndDrawFloatingText(void) {
    float dt = CP_System_GetDt();
    CP_Settings_TextSize(32);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Font_Set(game_font);

    for (int i = 0; i < floating_text_count; i++) {
        FloatingText* ft = &floating_texts[i];
        ft->timer -= dt;

        if (ft->timer <= 0.0f) {
            // Remove this element by swapping with the last
            floating_texts[i] = floating_texts[floating_text_count - 1];
            floating_text_count--;
            i--; // Re-check the new element at this index
            continue;
        }

        // Update position (float up)
        ft->pos.y -= 20.0f * dt;

        // Set color with fade-out
        CP_Color color = ft->color;
        color.a = (int)(255.0f * ft->timer); // Fade out
        CP_Settings_Fill(color);

        // Draw
        CP_Font_DrawText(ft->text, ft->pos.x, ft->pos.y);
    }
}
static bool AllEnemiesDefeated(void) {
    if (!current_enemies) return false; // Safety check
    for (int i = 0; i < current_enemy_count; i++) {
        if (current_enemies[i].alive && current_enemies[i].health > 0) {
            return false;
        }
    }
    return true;
}
void DrawStageClearBanner(void) {
    CP_Settings_Fill(CP_Color_Create(255, 255, 0, 255));
    CP_Settings_TextSize(48);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Font_DrawText("STAGE CLEARED!", CP_System_GetWindowWidth() / 2.0f, CP_System_GetWindowHeight() / 2.0f);
}

static void SyncDeckToArray(bool clear_hand) {
    draw_pile_size = 0;
    discard_size = 0;
    if (clear_hand) {
        hand_size = 0;
    }

    CP_Vector deck_pos_center = CP_Vector_Set(deck_pos.x + CARD_W_INIT / 2.0f, deck_pos.y + CARD_H_INIT / 2.0f);

    for (int i = 0; i < player_deck.size && i < MAX_DECK_SIZE; i++) {
        draw_pile[i] = player_deck.cards[i];
        draw_pile[i].pos = deck_pos_center; // Set spawn pos
    }
    draw_pile_size = player_deck.size;

    ShuffleDeck(draw_pile, draw_pile_size);
}


void ResetStageState(void) {
    selected_enemy = 0; // Default to first enemy
    for (int i = 0; i < 16; i++) {
        enemy_hit_flash[i] = 0.0f;
        enemy_shield_flash[i] = 0.0f;
    }
    player_hit_flash = 0.0f;
    player_shield_flash = 0.0f;
    floating_text_count = 0;

    stage_cleared = false;
    reward_active = false;
    buff_reward_active = false;
    banner_timer = 0.0f;

    current_phase = PHASE_PLAYER;
    enemy_anim_offset_x = 0.0f;
    enemy_has_hit = false;

    player.shield = START_SHIELD; // Reset shield

    ResetReward(&reward_state);
    ResetBuffReward(&buff_reward_state);
}

void ResetGame(void) {
    player.health = START_HEALTH;
    player.max_health = START_HEALTH;
    player.attack = START_ATTACK;
    player.shield = START_SHIELD;

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

    hand_size = 0;
    discard_size = 0;
    draw_pile_size = 0;

    selected_card_index = -1;
    played_cards = 0;
    drawn = false;

    turn_num = 0;
    current_level = 1;
    current_enemy_count = 0;
    current_enemies = NULL;

    ResetStageState();
}

void LoadLevel(int level) {
    if (level > MAX_LEVEL) {
        CP_Engine_SetNextGameState(Victory_Init, Victory_Update, Victory_Exit);
        return;
    }

    player.checkpoint_level = level;

    current_enemies = NULL;
    current_enemy_count = 0;
    current_level = level;

    // --- MODIFIED: Discard hand and reset turn flags ---
    // This logic only runs when transitioning levels (not on new game/restart)
    if (turn_num > 0) {
        // Move all cards from hand to discard pile
        for (int i = 0; i < hand_size; i++) {
            discard[discard_size] = hand[i];
            discard_size++;
        }
        hand_size = 0;

        // Move all cards from discard pile TO the draw pile
        CP_Vector deck_pos_center = CP_Vector_Set(deck_pos.x + CARD_W_INIT / 2.0f, deck_pos.y + CARD_H_INIT / 2.0f);
        for (int i = 0; i < discard_size; i++) {
            draw_pile[draw_pile_size] = discard[i];
            draw_pile[draw_pile_size].pos = deck_pos_center;
            draw_pile_size++;
        }
        discard_size = 0;

        // Shuffle the (now full) draw pile
        ShuffleDeck(draw_pile, draw_pile_size);
    }

    // Reset turn-specific flags
    drawn = false;
    turn_num = 0;
    played_cards = 0;
    // --- END MODIFICATION ---

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

    // Revive and reset enemies for the level
    if (current_enemies && current_enemy_count > 0) {
        for (int i = 0; i < current_enemy_count; i++) {
            current_enemies[i].health = current_enemies[i].max_health;
            current_enemies[i].shield = 0; // Reset shield
            current_enemies[i].alive = true;
            current_enemies[i].has_used_special = false;
            if (current_enemies[i].enrages) {
                current_enemies[i].attack = current_enemies[i].max_attack;
            }
        }
    }

    ResetStageState();
}

static void HandleEnemySelection(void) {
    if (!current_enemies || current_enemy_count == 0) {
        selected_enemy = -1; // No enemies to select
        return;
    }

    int current_selection = selected_enemy;
    int original_selection = selected_enemy;
    bool found_new_target = false;

    // --- FIX: Check array bounds before access ---
    if (original_selection < 0 || original_selection >= current_enemy_count || (current_enemies && !current_enemies[original_selection].alive)) {
        // Try to find a living enemy starting from the beginning
        for (int i = 0; i < current_enemy_count; ++i) {
            if (current_enemies[i].alive) {
                selected_enemy = i;
                found_new_target = true;
                break;
            }
        }
        if (found_new_target) return; // Found a new target automatically
        else { // No living enemies left
            selected_enemy = -1;
            return;
        }
    }


    // Manual selection with LEFT/RIGHT keys (only if current target is alive)
    if (CP_Input_KeyTriggered(KEY_LEFT)) {
        current_selection--;
        while (current_selection != original_selection) {
            if (current_selection < 0) current_selection = current_enemy_count - 1; // Wrap around
            if (current_enemies[current_selection].alive) {
                selected_enemy = current_selection;
                return;
            }
            if (current_selection == original_selection) break; // Full loop
            current_selection--;
        }
    }
    if (CP_Input_KeyTriggered(KEY_RIGHT)) {
        current_selection++;
        while (current_selection != original_selection) {
            if (current_selection >= current_enemy_count) current_selection = 0; // Wrap around
            if (current_enemies[current_selection].alive) {
                selected_enemy = current_selection;
                return;
            }
            if (current_selection == original_selection) break; // Full loop
            current_selection++;
        }
    }
}

static int UpdateStageClear(void) {
    if (!stage_cleared && !reward_active && !buff_reward_active && AllEnemiesDefeated() && current_enemy_count > 0) {
        stage_cleared = true;
        banner_timer = 2.0f;
    }
    if (stage_cleared) {
        if (banner_timer > 0.0f) {
            banner_timer -= CP_System_GetDt();
            DrawStageClearBanner();

            UpdateAndDrawFloatingText();

            return 1;
        }
        else {
            stage_cleared = false;
            if (current_level == 3 || current_level == 6 || current_level == 9) {
                buff_reward_active = true;
                GenerateBuffOptions(&buff_reward_state, current_level);
            }
            else {
                reward_active = true;
                GenerateRewardOptions(&reward_state, &player);
            }
            return 1;
        }
    }

    if (reward_active) {
        UpdateReward(&reward_state, &player_deck, &player);
        DrawReward(&reward_state, &player);
        if (reward_state.reward_claimed) {
            reward_active = false;
            SyncDeckToArray(true); // Clears hand, rebuilds draw pile
            LoadLevel(current_level + 1);
        }
        return 1;
    }

    if (buff_reward_active) {
        UpdateBuffReward(&buff_reward_state, &player);
        DrawBuffReward(&buff_reward_state);
        if (buff_reward_state.reward_claimed) {
            buff_reward_active = false;
            SyncDeckToArray(true); // Clears hand, rebuilds draw pile
            LoadLevel(current_level + 1);
        }
        return 1;
    }

    return 0;
}

static void DrawEntity(const char* name, int health, int max_health,
    int attack, int shield, float x, float y, float w, float h,
    CP_Color model_color, int is_selected, float hit_flash_timer, float shield_flash_timer)
{
    int clamped_health = (health < 0) ? 0 : health;
    if (clamped_health > max_health) clamped_health = max_health;

    float ratio = (max_health > 0) ? ((float)clamped_health / (float)max_health) : 0.0f;

    // 1. Draw Selection Border (if selected)
    CP_Settings_RectMode(CP_POSITION_CORNER);
    if (is_selected) {
        CP_Settings_StrokeWeight(4.0f);
        CP_Settings_Stroke(CP_Color_Create(255, 255, 0, 255));
        CP_Graphics_DrawRect(x - 4.0f, y - 4.0f, w + 8.0f, h + 8.0f);
    }

    // 2. Draw Hit/Shield Flash (behind model)
    if (hit_flash_timer > 0.0f) {
        CP_Settings_Fill(CP_Color_Create(255, 0, 0, 150 + (int)(hit_flash_timer * 105.0f)));
        CP_Graphics_DrawRect(x - 8.0f, y - 8.0f, w + 16.0f, h + 16.0f);
    }
    else if (shield_flash_timer > 0.0f) {
        CP_Settings_Fill(CP_Color_Create(0, 150, 255, 150 + (int)(shield_flash_timer * 105.0f)));
        CP_Graphics_DrawRect(x - 8.0f, y - 8.0f, w + 16.0f, h + 16.0f);
    }
    CP_Settings_StrokeWeight(0);

    // 3. Draw Model
    CP_Settings_Fill(model_color);
    CP_Graphics_DrawRect(x, y, w, h);

    // 4. Draw HP Bar
    float bar_h = 16.0f;
    float bar_y = y + h + 4.0f;
    CP_Settings_Fill(CP_Color_Create(70, 70, 70, 255));
    CP_Graphics_DrawRect(x, bar_y, w, bar_h);

    CP_Color hp_color = (ratio > 0.6f) ? CP_Color_Create(0, 200, 0, 255)
        : (ratio > 0.3f) ? CP_Color_Create(255, 200, 0, 255)
        : CP_Color_Create(200, 0, 0, 255);
    CP_Settings_Fill(hp_color);
    CP_Graphics_DrawRect(x, bar_y, w * ratio, bar_h);

    // 5. Draw Text (Name & HP)
    CP_Font_Set(game_font);
    CP_Settings_TextSize(20);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Font_DrawText(name, x + w * 0.5f, y - 20.0f);

    char hp_text[32];
    snprintf(hp_text, sizeof(hp_text), "%d/%d", clamped_health, max_health);
    CP_Settings_TextSize(16);
    CP_Font_DrawText(hp_text, x + w * 0.5f, bar_y + bar_h * 0.5f + 2.0f);

    // 6. Draw Stats (ATK & Shield)
    CP_Settings_TextSize(18);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
    float stats_y = bar_y + bar_h + 10.0f;

    // ATK (Red)
    CP_Settings_Fill(CP_Color_Create(255, 80, 80, 255));
    char atk_text[32]; snprintf(atk_text, sizeof(atk_text), "ATK: %d", attack);
    CP_Font_DrawText(atk_text, x, stats_y);

    // Shield (Blue) - Draw dynamically
    if (shield > 0) {
        CP_Settings_Fill(CP_Color_Create(80, 120, 255, 255));
        char shield_text[32]; snprintf(shield_text, sizeof(shield_text), "SHD: %d", shield);
        CP_Font_DrawText(shield_text, x + (w * 0.5f) + 10.0f, stats_y);
    }
}


void UpdateEnemyTurn(void) {
    if (!current_enemies) {
        current_phase = PHASE_PLAYER;
        return;
    }

    float dt = CP_System_GetDt();
    enemy_turn_timer += dt;

    // Check if all enemies have acted
    if (enemy_action_index >= current_enemy_count) {
        current_phase = PHASE_PLAYER;
        turn_num++;
        played_cards = 0;
        drawn = false;
        enemy_anim_offset_x = 0.0f;
        enemy_has_hit = false;

        if (current_enemies) {
            for (int i = 0; i < current_enemy_count; i++) {
                if (current_enemies[i].alive && current_enemies[i].enrages) {
                    current_enemies[i].attack += current_enemies[i].enrage_amount;

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

    Enemy* e = &current_enemies[enemy_action_index];

    // Skip dead enemies
    if (!e->alive) {
        enemy_action_index++;
        enemy_turn_timer = 0.0f;
        return;
    }

    if (e->is_necromancer && !e->has_used_special) {
        bool revived_ally = false;
        for (int i = 0; i < current_enemy_count; i++) {
            if (i != enemy_action_index && !current_enemies[i].alive) {
                revived_ally = true;
                current_enemies[i].alive = true;
                current_enemies[i].health = current_enemies[i].max_health / 2;

                float ww = (float)CP_System_GetWindowWidth();
                float wh = (float)CP_System_GetWindowHeight();
                float enemy_width = 120.0f;
                float spacing = 40.0f;
                float total_width = (float)current_enemy_count * enemy_width + ((float)current_enemy_count - 1.0f) * spacing;
                float start_x = ww - total_width - 200.0f;
                float enemy_x = start_x + (float)i * (enemy_width + spacing);
                float enemy_y = wh / 2.0f - 160.0f / 2.0f;
                SpawnFloatingText("Revived!", CP_Vector_Set(enemy_x + 60.0f, enemy_y), CP_Color_Create(80, 255, 80, 255));

                e->has_used_special = true;

                enemy_action_index++;
                enemy_turn_timer = 0.0f;
                return;
            }
        }
    }

    if (enemy_turn_timer < 0.3f) { // Lunge
        float ratio = enemy_turn_timer / 0.3f;
        enemy_anim_offset_x = -200.0f * ratio;
    }
    else if (!enemy_has_hit) { // Hit
        enemy_has_hit = true;
        enemy_anim_offset_x = -200.0f;

        int damage_to_deal = e->attack;
        int damage_blocked = 0;
        int damage_dealt = 0;

        if (player.shield > 0) {
            damage_blocked = (damage_to_deal <= player.shield) ? damage_to_deal : player.shield;
            player.shield -= damage_blocked;
            player_shield_flash = 0.2f;
        }

        damage_dealt = damage_to_deal - damage_blocked;
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
    else if (enemy_turn_timer < 0.6f) { // Retreat
        float ratio = (enemy_turn_timer - 0.3f) / 0.3f;
        enemy_anim_offset_x = -200.0f * (1.0f - ratio);
    }
    else { // Move to next enemy
        enemy_action_index++;
        enemy_turn_timer = 0.0f;
        enemy_anim_offset_x = 0.0f;
        enemy_has_hit = false;
    }
}

// ---------------------------------------------------------
// 3. INITIALIZATION
// ---------------------------------------------------------

void Game_Init(void)
{
    // --- 1. MODIFIED: Load assets and init states (ALWAYS RUN) ---
    game_bg = CP_Image_Load("Assets/background.jpg");
    game_font = CP_Font_Load("Assets/Exo2-Regular.ttf");
    if (game_font != 0) { CP_Font_Set(game_font); CP_Settings_TextSize(24); }

    deck_pos = CP_Vector_Set(50, 600);
    discard_pos = CP_Vector_Set(200, 600);

    InitReward(&reward_state);
    InitBuffReward(&buff_reward_state);

    // --- 2. MODIFIED: Check for checkpoint or new game ---
    if (g_is_restarting_from_checkpoint) {
        // --- RESTART FROM CHECKPOINT ---
        g_is_restarting_from_checkpoint = false;

        // Reset volatile stats
        player.health = player.max_health;
        player.shield = START_SHIELD;

        // Reset deck for fresh level start
        hand_size = 0;
        discard_size = 0;
        SyncDeckToArray(true); // Populates draw_pile from master deck

        // Reset turn flags
        drawn = false;
        turn_num = 0;
        played_cards = 0;
        selected_card_index = -1;

        // Load the checkpoint level
        LoadLevel(player.checkpoint_level);
    }
    else {
        // --- START NEW GAME ---
        ResetGame(); // Resets all player stats
        InitDeck(&player_deck); // Creates master deck
        SyncDeckToArray(true); // Fills draw pile, hand_size = 0
        LoadLevel(current_level); // Loads level 1 assets
    }
}

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

void Game_Update(void) {
    float dt = CP_System_GetDt();
    float ww = (float)CP_System_GetWindowWidth();
    float wh = (float)CP_System_GetWindowHeight();
    float mx = (float)CP_Input_GetMouseX();
    float my = (float)CP_Input_GetMouseY();
    int mouse_clicked = CP_Input_MouseClicked();
    bool hand_needs_realignment = false;

    // --- 1. Draw Background ---
    CP_Graphics_ClearBackground(CP_Color_Create(20, 25, 28, 255));
    if (game_bg) CP_Image_Draw(game_bg, ww * 0.5f, wh * 0.5f, ww, wh, 255);

    // --- 2. Check for Reward/Stage Clear Screen ---
    if (UpdateStageClear() == 1) {
        return;
    }

    // --- 3. Check for Game Over ---
    if (player.health <= 0) {
        CP_Engine_SetNextGameState(GameOver_Init, GameOver_Update, GameOver_Exit);
        return;
    }

    // --- 4. Update Flash Timers ---
    if (player_hit_flash > 0.0f) player_hit_flash -= dt;
    if (player_shield_flash > 0.0f) player_shield_flash -= dt;
    if (current_enemies) {
        for (int i = 0; i < current_enemy_count; i++) {
            if (enemy_hit_flash[i] > 0.0f) enemy_hit_flash[i] -= dt;
            if (enemy_shield_flash[i] > 0.0f) enemy_shield_flash[i] -= dt;
        }
    }

    // --- 5. Phase Logic ---
    if (current_phase == PHASE_ENEMY) {
        UpdateEnemyTurn();
    }
    else {
        // --- Player Turn Logic ---
        int living_enemies = 0;
        int last_living_index = -1;
        if (current_enemies) {
            for (int i = 0; i < current_enemy_count; i++) {
                if (current_enemies[i].alive) {
                    living_enemies++;
                    last_living_index = i;
                }
            }
        }

        // --- MODIFIED: Automatic selection of next living enemy if current is dead ---
        if (selected_enemy == -1 || (current_enemies && !current_enemies[selected_enemy].alive)) {
            HandleEnemySelection();
        }
        else if (living_enemies > 1) { // Manual select only if multiple enemies AND current target is alive
            HandleEnemySelection();
        }

        // --- MODIFIED: Removed KEY_Q to quit ---
    }

    // --- 6. Draw Player & Enemies ---
    DrawEntity("Player", player.health, player.max_health,
        player.attack, player.shield,
        100.0f, wh / 2.0f - 100.0f, 150.0f, 200.0f,
        CP_Color_Create(50, 50, 150, 255),
        0, // Player is never "selected"
        player_hit_flash, player_shield_flash);

    if (current_enemies && current_enemy_count > 0) { // Safety check
        float enemy_width = 120.0f;
        float enemy_height = 160.0f;
        float spacing = 40.0f;
        float total_width = (float)current_enemy_count * enemy_width + ((float)current_enemy_count - 1.0f) * spacing;
        float start_x = ww - total_width - 200.0f;
        float y = wh / 2.0f - enemy_height / 2.0f;

        for (int i = 0; i < current_enemy_count; i++) {
            Enemy* e = &current_enemies[i];
            float x = start_x + i * (enemy_width + spacing);

            if (current_phase == PHASE_ENEMY && i == enemy_action_index && e->alive) {
                x += enemy_anim_offset_x;
            }

            CP_Color col = e->alive ? CP_Color_Create(120, 120, 120, 255) : CP_Color_Create(80, 80, 80, 150);

            DrawEntity(e->name, e->health, e->max_health, e->attack, e->shield,
                x, y, enemy_width, enemy_height, col,
                (selected_enemy == i), enemy_hit_flash[i], enemy_shield_flash[i]);
        }
    }

    // --- 7. Draw HUD & Floating Text ---
    char hud_text[128];
    snprintf(hud_text, sizeof(hud_text), "Level %d | Turn %d", current_level, turn_num + 1);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
    CP_Settings_Fill(CP_Color_Create(255, 255, 0, 255));
    CP_Font_Set(game_font);
    CP_Settings_TextSize(30);
    CP_Font_DrawText(hud_text, 20, 35);

    // --- MODIFIED: Draw Death Counter ---
    char death_text[32];
    snprintf(death_text, sizeof(death_text), "Restarts: %d", player.death_count);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_RIGHT, CP_TEXT_ALIGN_V_TOP);
    CP_Font_DrawText(death_text, ww - 20.0f, 35.0f);
    // --- END MODIFICATION ---

    // --- NEW: Draw Active Buffs ---
    float buff_text_y = 70.0f;
    CP_Settings_TextSize(18);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP); // <-- FIX: Reset alignment
    CP_Settings_Fill(CP_Color_Create(0, 255, 255, 255)); // Cyan color for buffs
    if (player.has_lifesteal) {
        CP_Font_DrawText("Buff: Vampiric Strike (50% Lifesteal)", 20, buff_text_y);
        buff_text_y += 25.0f;
    }
    if (player.has_desperate_draw) {
        CP_Font_DrawText("Buff: Sudden Insight (Draw 3 on empty hand)", 20, buff_text_y);
        buff_text_y += 25.0f;
    }
    if (player.has_divine_strike) {
        CP_Font_DrawText("Buff: Divine Strike (Heal deals 50% AOE damage)", 20, buff_text_y);
        buff_text_y += 25.0f;
    }
    if (player.has_shield_boost) {
        CP_Font_DrawText("Buff: Reinforce (25% Bonus Shield)", 20, buff_text_y); // <-- Text updated
        buff_text_y += 25.0f;
    }
    if (player.has_attack_boost_35) {
        CP_Font_DrawText("Buff: Power Infusion (35% Bonus Attack)", 20, buff_text_y);
        buff_text_y += 25.0f;
    }
    if (player.has_heal_boost_35) {
        CP_Font_DrawText("Buff: Holy Infusion (35% Bonus Heal)", 20, buff_text_y);
        buff_text_y += 25.0f;
    }
    if (player.has_shield_boost_35) {
        CP_Font_DrawText("Buff: Barrier Infusion (35% Bonus Shield)", 20, buff_text_y);
        buff_text_y += 25.0f;
    }
    // --- END NEW ---

    UpdateAndDrawFloatingText();

    // --- 8. Card Logic: Purge Discarding Cards ---
    for (int i = 0; i < hand_size; i++) {
        if (hand[i].is_discarding && !hand[i].is_animating) {

            discard[discard_size] = hand[i];
            discard[discard_size].is_discarding = false; // Reset flag
            discard_size++;

            for (int j = i; j < hand_size - 1; j++) {
                hand[j] = hand[j + 1];
            }

            hand_size--;
            hand_needs_realignment = true;
            i--;
        }
    }
    if (hand_needs_realignment) {
        SetHandPos(hand, hand_size);
        // --- MODIFIED: Auto-select first card if selection is lost ---
        if (selected_card_index >= hand_size) {
            selected_card_index = hand_size - 1;
        }
        if (selected_card_index < 0 && hand_size > 0) {
            selected_card_index = 0;
        }
    }

    // --- 9. Card Logic: Draw / Recycle ---
    CP_Settings_RectMode(CP_POSITION_CORNER);
    CP_Settings_Stroke(CP_Color_Create(0, 0, 0, 255));
    CP_Settings_Fill(CP_Color_Create(145, 145, 145, 255));
    CP_Graphics_DrawRect(deck_pos.x, deck_pos.y, CARD_W_INIT, CARD_H_INIT);
    CP_Graphics_DrawRect(discard_pos.x, discard_pos.y, CARD_W_INIT, CARD_H_INIT);

    CP_Font_Set(game_font);
    CP_Settings_TextSize(24);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Font_DrawText("Draw Pile", deck_pos.x + (CARD_W_INIT / 2.0f), deck_pos.y - 30.0f);
    CP_Font_DrawText("Discard", discard_pos.x + (CARD_W_INIT / 2.0f), discard_pos.y - 30.0f);

    char count_text[16];
    snprintf(count_text, sizeof(count_text), "%d", draw_pile_size);
    CP_Font_DrawText(count_text, deck_pos.x + (CARD_W_INIT / 2.0f), deck_pos.y + (CARD_H_INIT / 2.0f));
    snprintf(count_text, sizeof(count_text), "%d", discard_size);
    CP_Font_DrawText(count_text, discard_pos.x + (CARD_W_INIT / 2.0f), discard_pos.y + (CARD_H_INIT / 2.0f));


    if (!drawn) {
        if (turn_num == 0) { // First turn of a level
            for (int i = 0; i < 3; ++i) { // Draw 3 on first turn
                if (draw_pile_size == 0) { // Check if we need to recycle
                    RecycleDeck(discard, draw_pile, &discard_size, &draw_pile_size);
                    if (draw_pile_size == 0) break; // No cards left anywhere
                }
                if (hand_size < MAX_HAND_SIZE) {
                    DealFromDeck(draw_pile, &hand[hand_size], &draw_pile_size, &hand_size);
                }
            }
        }
        else { // Subsequent turns
            int cards_to_draw = 2; // --- MODIFIED: Always draw 2 cards ---
            if (player.has_desperate_draw && GetActiveHandSize() == 0) {
                cards_to_draw = 3; // Buff still works
            }

            for (int i = 0; i < cards_to_draw; ++i) {
                if (draw_pile_size == 0) { // Check if we need to recycle
                    RecycleDeck(discard, draw_pile, &discard_size, &draw_pile_size);
                    if (draw_pile_size == 0) break; // No cards left anywhere
                }
                if (hand_size < MAX_HAND_SIZE) {
                    DealFromDeck(draw_pile, &hand[hand_size], &draw_pile_size, &hand_size);
                }
            }
        }
        SetHandPos(hand, hand_size);
        drawn = true;

        // --- MODIFIED: Auto-select first card ---
        if (hand_size > 0) {
            selected_card_index = 0;
        }
        else {
            selected_card_index = -1;
        }
    }

    if (draw_pile_size == 0 && hand_size > 0) { // Don't recycle if hand is empty, wait for draw
        RecycleDeck(discard, draw_pile, &discard_size, &draw_pile_size);
    }

    // --- 10. Draw Hand & Button ---
    for (int i = 0; i < hand_size; ++i) {
        if (hand[i].is_animating) AnimateMoveCard(&hand[i]);
        if (i == selected_card_index) {
            CP_Settings_Stroke(CP_Color_Create(255, 255, 0, 255));
            CP_Settings_StrokeWeight(3.0f);
        }
        else {
            CP_Settings_Stroke(CP_Color_Create(0, 0, 0, 255));
            CP_Settings_StrokeWeight(1.0f);
        }
        DrawCard(&hand[i]);
    }

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

        // --- MODIFIED: Update button text for new keybinds ---
        if (selected_card_index >= 0) {
            CP_Font_DrawText("Use Card (S)", select_btn_x, select_btn_y);
        }
        else {
            CP_Font_DrawText("End Turn (Enter)", select_btn_x, select_btn_y);
        }
        // --- END MODIFICATION ---
    }

    // --- 11. MODIFIED: Reworked Click & Input Logic ---
    bool card_played_this_frame = false;
    bool clicked_on_card_in_hand = false;

    // --- A. Handle Mouse Clicks ---
    if (current_phase == PHASE_PLAYER && mouse_clicked) {

        // 1. Check Card Selection in hand
        for (int i = 0; i < hand_size; ++i) {
            if (hand[i].is_discarding) continue;
            if (IsAreaClicked(hand[i].pos.x, hand[i].pos.y, hand[i].card_w, hand[i].card_h, mx, my)) {
                SelectCard(i, &selected_card_index, hand);
                clicked_on_card_in_hand = true;
                break;
            }
        }

        // 2. Check Click-to-Play on Player (Heal or Shield)
        if (!clicked_on_card_in_hand && selected_card_index >= 0 &&
            (hand[selected_card_index].type == Heal || hand[selected_card_index].type == Shield)) {

            float player_x = 100.0f;
            float player_y = wh / 2.0f - 100.0f;
            float player_w = 150.0f;
            float player_h = 200.0f;
            float player_center_x = player_x + player_w / 2.0f;
            float player_center_y = player_y + player_h / 2.0f;

            if (IsAreaClicked(player_center_x, player_center_y, player_w, player_h, mx, my)) {
                card_played_this_frame = true; // Set flag to play the card
            }
        }

        // 3. Check Click-to-Play on Enemy (Attack)
        if (!clicked_on_card_in_hand && selected_card_index >= 0 &&
            (hand[selected_card_index].type == Attack)) {

            if (current_enemies && current_enemy_count > 0) { // Safety check
                float enemy_width = 120.0f;
                float enemy_height = 160.0f;
                float spacing = 40.0f;
                float total_width = (float)current_enemy_count * enemy_width + ((float)current_enemy_count - 1.0f) * spacing;
                float start_x = ww - total_width - 200.0f;
                float y = wh / 2.0f - enemy_height / 2.0f;

                for (int i = 0; i < current_enemy_count; i++) {
                    Enemy* e = &current_enemies[i];
                    float x = start_x + i * (enemy_width + spacing);

                    if (e->alive && IsAreaClicked(x + enemy_width / 2.0f, y + enemy_height / 2.0f, enemy_width, enemy_height, mx, my)) {
                        selected_enemy = i; // Set this as the target
                        card_played_this_frame = true; // Set flag to play the card
                        break;
                    }
                }
            }
        }

        // 4. Check Button Click
        if (!clicked_on_card_in_hand && !card_played_this_frame && IsAreaClicked(select_btn_x, select_btn_y, SELECT_BTN_W, SELECT_BTN_H, mx, my)) {
            if (selected_card_index >= 0) {
                card_played_this_frame = true; // Play card
            }
            else {
                // End Turn
                current_phase = PHASE_ENEMY;
                enemy_action_index = 0;
                enemy_turn_timer = 0.0f;
                enemy_anim_offset_x = 0.0f;
                selected_card_index = -1;
            }
        }
    }

    // --- B. Handle Keyboard Input ---
    if (current_phase == PHASE_PLAYER) {
        // Handle A/D for card cycling
        if (hand_size > 0) {
            if (CP_Input_KeyTriggered(KEY_A)) {
                selected_card_index--;
                if (selected_card_index < 0) selected_card_index = hand_size - 1;
            }
            if (CP_Input_KeyTriggered(KEY_D)) {
                selected_card_index++;
                if (selected_card_index >= hand_size) selected_card_index = 0;
            }
        }

        // --- MODIFIED: Use 'S' key ---
        if (CP_Input_KeyTriggered(KEY_S)) {
            if (selected_card_index >= 0 && !hand[selected_card_index].is_discarding) {
                card_played_this_frame = true;
            }
        }

        // Handle Enter for End Turn (Exclusively)
        if (CP_Input_KeyTriggered(KEY_ENTER)) {
            current_phase = PHASE_ENEMY;
            enemy_action_index = 0;
            enemy_turn_timer = 0.0f;
            enemy_anim_offset_x = 0.0f;
            selected_card_index = -1;
        }
    }


    // --- C. Process Card Play Logic (if flagged) ---
    if (current_phase == PHASE_PLAYER && card_played_this_frame) {
        if (selected_card_index >= 0 && !hand[selected_card_index].is_discarding) {

            Card* card = &hand[selected_card_index];
            Enemy* target_enemy = (current_enemies && selected_enemy >= 0 && selected_enemy < current_enemy_count) ? &current_enemies[selected_enemy] : NULL;
            bool has_valid_target = (target_enemy && target_enemy->alive);

            bool can_play_shield = (card->type == Shield); // Includes Shield Bash
            bool can_play_cleave = (card->type == Attack && card->effect == CLEAVE);
            bool can_play_heal = (card->type == Heal);
            bool can_play_attack = (card->type == Attack && card->effect != CLEAVE); // Needs valid target

            // --- MODIFIED: Check for valid target only when needed ---
            if (can_play_heal || can_play_shield || (can_play_attack && has_valid_target) || (can_play_cleave && has_valid_target))
            {
                // --- CRITICAL FIX: Pass 'selected_card_index' as an int ---
                int power = UseCard(hand, selected_card_index, &hand_size, &player, target_enemy);
                int total_damage_dealt_this_card = 0;

                // --- ATTACK LOGIC ---
                if (card->type == Attack) {
                    int damage_to_deal = power + player.attack_bonus;
                    if (player.has_attack_boost_35) {
                        damage_to_deal = (int)(damage_to_deal * 1.35f);
                    }
                    if (card->effect == CLEAVE) {
                        if (damage_to_deal <= 0) damage_to_deal = 1;
                        for (int i = 0; i < current_enemy_count; i++) {
                            if (current_enemies[i].alive) {
                                // (Rest of Cleave logic...)
                                Enemy* cleave_target = &current_enemies[i];
                                int damage_blocked = 0;
                                int damage_dealt = 0;
                                if (cleave_target->shield > 0) {
                                    damage_blocked = (damage_to_deal <= cleave_target->shield) ? damage_to_deal : cleave_target->shield;
                                    cleave_target->shield -= damage_blocked;
                                    enemy_shield_flash[i] = 0.2f;
                                }
                                damage_dealt = damage_to_deal - damage_blocked;
                                if (damage_dealt > 0) {
                                    cleave_target->health -= damage_dealt;
                                    enemy_hit_flash[i] = 0.2f;
                                    total_damage_dealt_this_card += damage_dealt;
                                    float enemy_width = 120.0f;
                                    float spacing = 40.0f;
                                    float total_width = (float)current_enemy_count * enemy_width + ((float)current_enemy_count - 1.0f) * spacing;
                                    float start_x = ww - total_width - 200.0f;
                                    float enemy_x = start_x + (float)i * (enemy_width + spacing);
                                    float enemy_y = wh / 2.0f - 160.0f / 2.0f;
                                    char dmg_text[16];
                                    snprintf(dmg_text, sizeof(dmg_text), "-%d", damage_dealt);
                                    SpawnFloatingText(dmg_text, CP_Vector_Set(enemy_x + 60.0f, enemy_y - 30.0f), CP_Color_Create(255, 80, 80, 255));
                                }
                                if (cleave_target->health <= 0) cleave_target->alive = false;
                            }
                        }
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
                    else { // Single-Target Attack
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
                            if (player.has_lifesteal) {
                                int lifesteal_amount = (int)(damage_dealt * 0.50f);
                                if (lifesteal_amount < 1 && damage_dealt > 0) lifesteal_amount = 1;
                                if (lifesteal_amount > 0) {
                                    player.health += lifesteal_amount;
                                    if (player.health > player.max_health) player.health = player.max_health;
                                    char heal_text[16];
                                    snprintf(heal_text, sizeof(heal_text), "+%d", lifesteal_amount);
                                    SpawnFloatingText(heal_text, CP_Vector_Set(175.0f, wh / 2.0f - 30.0f), CP_Color_Create(80, 255, 80, 255));
                                }
                            }
                            float enemy_width = 120.0f;
                            float spacing = 40.0f;
                            float total_width = (float)current_enemy_count * enemy_width + ((float)current_enemy_count - 1.0f) * spacing;
                            float start_x = ww - total_width - 200.0f;
                            float enemy_x = start_x + (float)selected_enemy * (enemy_width + spacing);
                            float enemy_y = wh / 2.0f - 160.0f / 2.0f;
                            char text[16];
                            snprintf(text, sizeof(text), "-%d", damage_dealt);
                            SpawnFloatingText(text, CP_Vector_Set(enemy_x + 60.0f, enemy_y), CP_Color_Create(255, 80, 80, 255));
                        }
                        else {
                            float enemy_width = 120.0f;
                            float spacing = 40.0f;
                            float total_width = (float)current_enemy_count * enemy_width + ((float)current_enemy_count - 1.0f) * spacing;
                            float start_x = ww - total_width - 200.0f;
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
                    char text[16];
                    snprintf(text, sizeof(text), "+%d", heal_amount);
                    SpawnFloatingText(text, CP_Vector_Set(175.0f, wh / 2.0f), CP_Color_Create(80, 255, 80, 255));
                    if (player.has_divine_strike || card->effect == DIVINE_STRIKE_EFFECT) {
                        int divine_damage = heal_amount / 2;
                        if (divine_damage < 1 && heal_amount > 0) divine_damage = 1;
                        if (divine_damage > 0 && current_enemies) {
                            for (int i = 0; i < current_enemy_count; i++) {
                                if (current_enemies[i].alive) {
                                    // (Rest of Divine Strike logic...)
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
                                        float enemy_width = 120.0f;
                                        float spacing = 40.0f;
                                        float total_width = (float)current_enemy_count * enemy_width + ((float)current_enemy_count - 1.0f) * spacing;
                                        float start_x = ww - total_width - 200.0f;
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
                    char text[16];
                    snprintf(text, sizeof(text), "+%d", shield_amount);
                    SpawnFloatingText(text, CP_Vector_Set(175.0f, wh / 2.0f), CP_Color_Create(80, 80, 255, 255));
                    if (card->effect == SHIELD_BASH) {
                        int damage_to_deal = (int)(player.shield * 0.75f);
                        if (damage_to_deal <= 0 && player.shield > 0) damage_to_deal = 1;
                        for (int i = 0; i < current_enemy_count; i++) {
                            if (current_enemies[i].alive) {
                                // (Rest of Shield Bash logic...)
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
                                    float enemy_width = 120.0f;
                                    float spacing = 40.0f;
                                    float total_width = (float)current_enemy_count * enemy_width + ((float)current_enemy_count - 1.0f) * spacing;
                                    float start_x = ww - total_width - 200.0f;
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

                // --- Discard Card ---
                ++played_cards;
                card->is_discarding = true;
                card->is_animating = true;
                card->target_pos = CP_Vector_Set(discard_pos.x + CARD_W_INIT / 2.0f, discard_pos.y + CARD_H_INIT / 2.0f);

                // --- MODIFIED: Auto-select next card ---
                if (hand_size > 0) {
                    selected_card_index = 0;
                }
                else {
                    selected_card_index = -1;
                }
            }
        }
    }

    // --- 12. Check for Auto-End of Player Turn ---
    if (current_phase == PHASE_PLAYER && GetActiveHandSize() == 0 && played_cards > 0) {
        current_phase = PHASE_ENEMY;
        enemy_action_index = 0;
        enemy_turn_timer = 0.0f;
        enemy_anim_offset_x = 0.0f;
        selected_card_index = -1;
    }
}

void Game_Exit(void)
{
    CP_Image_Free(game_bg);
}