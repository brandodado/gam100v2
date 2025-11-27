#pragma once
#include "cprocessing.h"
#include <stdbool.h>
#include "levels.h"  // Get Enemy type

#define CARD_W_INIT 60
#define CARD_H_INIT 90
#define MAX_DECK_SIZE 25

// Forward declare Player (game.h will provide full definition later)
typedef struct Player Player;

// Card enums 
typedef enum {
	Attack,
	Heal,
	Shield,
} CardType;

typedef enum {
	None,
	Draw,
	Fire,
	Poison,
	SHIELD_BASH,
	CLEAVE,
	DIVINE_STRIKE_EFFECT
} CardEffect;

// Card struct
typedef struct Card {
	CP_Vector pos;
	CP_Vector target_pos;
	CardType type;
	CardEffect effect;
	int power;
	char description[200];
	float card_w;
	float card_h;
	bool is_animating;
	bool is_discarding;
} Card;

// Deck struct
typedef struct Deck {
	Card cards[MAX_DECK_SIZE];
	int size;
	int capacity;
} Deck;

// Global card catalogue
extern Card catalogue[50];
extern int catalogue_size;

// Card functions
void DrawCard(Card* handptr, Player* player_ptr);
void SelectCard(int index, int* selected);
float CalculateCardScale(int hand_size);
void SetHandPos(Card* hand, int hand_size);
void DealFromDeck(Deck* deck, Card* hand_slot, int* hand_size);
//int UseCard(Card* hand, int* selected_index, int* hand_size, Player* player_ptr, Enemy* enemy_ptr);
void RecycleDeck(Card* discard, Deck* deck, int* discard_size);
void AnimateMoveCard(Card* hand, float speed);
void ShuffleDeck(Deck* deck);
int LoadCatalogue(const char* fcat, Card* cat_arr, int max_size);

// Deck functions
void InitDeck(Deck* deck);
bool AddCardToDeck(Deck* deck, Card card);
bool RemoveCardFromDeck(Deck* deck, int index);
Card GetDeckCard(Deck* deck, int index);
bool IsDeckFull(Deck* deck);
bool IsDeckEmpty(Deck* deck);
int GetDeckSize(Deck* deck);
void ClearDeck(Deck* deck);