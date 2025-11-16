#pragma once
#include "card.h"

#define MAX_DECK_SIZE 25

// Deck struct to manage player's card collection
typedef struct {
    Card cards[MAX_DECK_SIZE];
    int size;
    int capacity;
} Deck;

// Initialize deck with starting cards
void InitDeck(Deck* deck);

// Add a card to deck
bool AddCardToDeck(Deck* deck, Card card);

// Remove a card from deck at a specific index
bool RemoveCardFromDeck(Deck* deck, int index);

// Get a card from deck (returns a copy)
Card GetDeckCard(Deck* deck, int index);

// Check if deck is full
bool IsDeckFull(Deck* deck);

// Check if deck is empty
bool IsDeckEmpty(Deck* deck);

// Get current size of deck
int GetDeckSize(Deck* deck);

// Clear deck
void ClearDeck(Deck* deck);