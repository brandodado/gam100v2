
#pragma once
#include "cprocessing.h"
#include <stdbool.h>
#include "levels.h"
// #include "game.h" // <-- REMOVED THIS to fix circular dependency

// --- NEW: Forward declarations ---
// This tells card.h that these types exist
// without needing to include the full header.
struct Player;
struct Enemy;
// --- END NEW ---


#define CARD_W_INIT 80.0f
#define CARD_H_INIT 120.0f


typedef enum {
    Attack,
    Heal
}CardType;

typedef enum {
    None,
    DOT,
    Draw
}CardEffect;

typedef struct {
    CP_Vector pos;
    CP_Vector target_pos;
    CardType type;
    CardEffect effect;
    float power;

    char title[64];
    char description_template[128];

    float card_w;
    float card_h;
    bool is_animating;
}Card;


// --- FUNCTION PROTOTYPES ---

// --- UPDATED: Added icon parameters ---
void DrawCard(Card * hand, CP_Image * type_icons, CP_Image * effect_icons);

// --- UPDATED: Removed extra 'hand' parameter ---
void SelectCard(int index, int* selected);

// --- UPDATED: Added deck, deck_size, and did_miss parameters ---
void UseCard(Card * hand, int* selected_index, int* hand_size, struct Player* player_ptr, struct Enemy* enemy_ptr, Card * deck, int* deck_size, bool did_miss);


// --- Deck/Hand Management (Moved from game.c) ---
float HandWidth(Card hand[], int size, float margin);
void SetHandPos(Card * hand, int hand_size);
void ShuffleDeck(Card * deck, int deck_size);
void DealFromDeck(Card * deck, Card * hand, int* deck_size, int* hand_size);
void DiscardCard(Card * hand, int* selected_index, int* hand_size, Card * discard, int* discard_size);
void RecycleDeck(Card * discard, Card * deck, int* discard_size, int* deck_size);
void AnimateMoveCard(Card * hand);