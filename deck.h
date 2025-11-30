// Management functions for the Deck structure.
#pragma once
#include "card.h"

// Initializes the deck with a standard set of starting cards.
void InitDeck(Deck* deck);

// Adds a single card to the deck. Returns true if successful, or false if the deck is full.
bool AddCardToDeck(Deck* deck, Card card);

// Removes the card at the specified index from the deck and shifts remaining cards. Returns true if successful.
bool RemoveCardFromDeck(Deck* deck, int index);

// Retrieves a copy of the card at the specified index from the deck. Returns a dummy card if the index is invalid.
Card GetDeckCard(Deck* deck, int index);

// Checks if the deck has reached its capacity. Returns true if full.
bool IsDeckFull(Deck* deck);

// Checks if the deck has no cards. Returns true if empty.
bool IsDeckEmpty(Deck* deck);

// Returns the current number of cards in the deck.
int GetDeckSize(Deck* deck);

// Resets the deck size to zero, effectively clearing it.
void ClearDeck(Deck* deck);