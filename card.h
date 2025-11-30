// Definitions and function prototypes for Card objects and interactions.
#pragma once
#include "cprocessing.h"
#include <stdbool.h>
#include "levels.h"

#define CARD_W_INIT 60
#define CARD_H_INIT 90
#define MAX_DECK_SIZE 25

// Forward declare Player
typedef struct Player Player;

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

typedef struct Deck {
	Card cards[MAX_DECK_SIZE];
	int size;
	int capacity;
} Deck;

extern Card catalogue[50];
extern int catalogue_size;

// Renders the visual representation of the card pointed to by handptr to the screen, using player context if needed.
void DrawCard(Card* hand);

// Toggles the selection state of the card at the specified index, updating the selected variable.
void SelectCard(int index, int* selected);

// Calculates and returns the scaling factor for cards to ensure they fit on screen based on the current hand_size.
float CalculateCardScale(int hand_size);

// Updates the target positions of all cards in the hand array for proper layout.
void SetHandPos(Card* hand, int hand_size);

// Moves a card from the deck to the specified hand_slot and increments the hand_size counter.
void DealFromDeck(Deck* deck, Card* hand_slot, int* hand_size);

// Moves all cards from the discard pile back into the deck, resets the discard_size, and shuffles the deck.
void RecycleDeck(Card* discard, Deck* deck, int* discard_size);

// Updates the card's current position to move towards its target position at the specified speed.
void AnimateMoveCard(Card* hand, float speed);

// Randomizes the order of cards currently in the deck.
void ShuffleDeck(Deck* deck);

// Loads card definitions from the text file (fcat) into the catalogue array (cat_arr). Returns the number of cards loaded.
int LoadCatalogue(const char* fcat, Card* cat_arr, int max_size);