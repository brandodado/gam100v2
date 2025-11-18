#pragma once
#include "cprocessing.h"
#include <stdbool.h>
#include "levels.h"
#include "game.h" // Include game.h for Player and Enemy structs

#define CARD_W_INIT 80
#define CARD_H_INIT 120

typedef enum {
	Attack,
	Heal,
	Shield,
}CardType;

typedef enum {
	None,
	Multihit,
	Draw,
	DOT,
	SHIELD_BASH,
	CLEAVE,
	DIVINE_STRIKE_EFFECT
}CardEffect;

typedef struct {
	CP_Vector pos;
	CP_Vector target_pos;
	CardType type;
	CardEffect effect;
	float power;
	char description[200];
	float card_w;
	float card_h;
	bool is_animating;
	bool is_discarding;
}Card;

void DrawCard(Card* handptr);

void SelectCard(int index, int* selected, Card hand[]);

float HandWidth(Card hand[], int size, float margin);
void SetHandPos(Card hand[], int hand_size);

void ShuffleDeck(Card* deck, int deck_size);

void DealFromDeck(Card deck[], Card* hand_slot, int* deck_size, int* hand_size);

// --- CRITICAL FIX: selected_index is an int, not an int* ---
int UseCard(Card hand[], int selected_index, int* hand_size, Player* player_ptr, Enemy* enemy_ptr);

// --- CRITICAL FIX: selected_index is an int, not an int* ---
void DiscardCard(Card hand[], int selected_index, int* hand_size, Card discard[], int* discard_size);

void RecycleDeck(Card discard[], Card deck[], int* discard_size, int* deck_size);

void AnimateMoveCard(Card* card_to_animate);