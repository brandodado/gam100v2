#pragma once
#include "cprocessing.h"
#include <stdbool.h>
#include "levels.h"
#include "game.h"


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
	CLEAVE,				// <-- MODIFIED: New effect
	DIVINE_STRIKE_EFFECT // <-- MODIFIED: New effect
}CardEffect;

typedef struct {
	CP_Vector pos;
	CP_Vector target_pos;
	CardType type;
	CardEffect effect;
	// float effect_power; 
	float power;
	char description[200];
	float card_w;
	float card_h;
	bool is_animating;
	bool is_discarding;
}Card;


Card MakeCard(CP_Vector pos, CardType type, CardEffect effect, float pow, char* desc, float w, float h, bool is_animating);

// 
void DrawCard(Card* handptr);

// highlight card
void SelectCard(int index, int* selected, Card hand[]);


void SetHandPos(Card hand[], int hand_size);

// pass top card from deck to hand increment hand size (either here or game.c) shift card to top
// decrement deck size
// dealing with removal from deck?
// take from deck[0] and reset per card draw?? card draw to hand [i]
void DealFromDeck(Card deck[], Card* hand_slot, int* deck_size, int* hand_size);


// use card
// --- MODIFIED: Changed from 'void' to 'int' ---
int UseCard(Card hand[], int* selected_index, int* hand_size, Player* player_ptr, Enemy* enemy_ptr);


void DiscardCard(Card hand[], int* selected_index, int* hand_size, Card discard[], int* discard_size);


void RecycleDeck(Card discard[], Card deck[], int* discard_size, int* deck_size);


void AnimateMoveCard(Card* hand);