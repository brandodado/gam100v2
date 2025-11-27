#include "deck.h"
#include "cprocessing.h"
#include <stdlib.h>
#include <string.h>

// Matching scale factor again
#define CARD_SCALE 1.5f

// This function builds the initial starting deck when a new game begins
void InitDeck(Deck* deck) {
    if (!deck) return;

    deck->size = 0;
    deck->capacity = MAX_DECK_SIZE;

    // Position for spawn (same as Draw Pile in game.c)
    CP_Vector deck_pos_topleft = CP_Vector_Set(50, 550);

    // Calculate center using the scale factor
    CP_Vector deck_pos_center = CP_Vector_Set(
        deck_pos_topleft.x + (CARD_W_INIT * CARD_SCALE) / 2.0f,
        deck_pos_topleft.y + (CARD_H_INIT * CARD_SCALE) / 2.0f
    );

    float final_w = CARD_W_INIT * CARD_SCALE;
    float final_h = CARD_H_INIT * CARD_SCALE;

    // Define our 3 basic starter cards
    Card attack = {
        deck_pos_center,
        CP_Vector_Set(0, 0),
        Attack,
        None,
        7,
        "Deal 7 Dmg.",
        final_w,
        final_h,
        false,
        false
    };

    Card heal = {
        deck_pos_center,
        CP_Vector_Set(0, 0),
        Heal,
        None,
        7,
        "Heal 7 HP.",
        final_w,
        final_h,
        false,
        false
    };

    Card shield = {
        deck_pos_center,
        CP_Vector_Set(0, 0),
        Shield,
        None,
        5,
        "Gain 5 Shield.",
        final_w,
        final_h,
        false,
        false
    };

    // Populate the deck
    // 6 Attacks, 4 Heals, 4 Shields = 14 Cards total
    for (int i = 0; i < 6; i++) {
        AddCardToDeck(deck, attack);
    }
    for (int i = 0; i < 4; i++) {
        AddCardToDeck(deck, heal);
    }
    for (int i = 0; i < 4; i++) {
        AddCardToDeck(deck, shield);
    }
}

// Safely adds a card to the end of the deck array
bool AddCardToDeck(Deck* deck, Card card) {
    if (!deck || deck->size >= deck->capacity) {
        return false; // Deck full!
    }

    deck->cards[deck->size] = card;
    deck->size++;
    return true;
}

// Removes a card at a specific index and shifts everything down to fill the gap
bool RemoveCardFromDeck(Deck* deck, int index) {
    if (!deck || index < 0 || index >= deck->size) {
        return false;
    }

    for (int i = index; i < deck->size - 1; i++) {
        deck->cards[i] = deck->cards[i + 1];
    }

    deck->size--;
    return true;
}

// Safe getter that returns a dummy card if index is invalid
Card GetDeckCard(Deck* deck, int index) {
    if (!deck || index < 0 || index >= deck->size) {
        Card default_card = {
            CP_Vector_Set(0, 0),
            CP_Vector_Set(0, 0),
            Attack,
            None,
            0,
            "Invalid",
            CARD_W_INIT * CARD_SCALE,
            CARD_H_INIT * CARD_SCALE,
            false,
            false
        };
        return default_card;
    }

    return deck->cards[index];
}

bool IsDeckFull(Deck* deck) {
    if (!deck) return true;
    return deck->size >= deck->capacity;
}

bool IsDeckEmpty(Deck* deck) {
    if (!deck) return true;
    return deck->size == 0;
}

int GetDeckSize(Deck* deck) {
    if (!deck) return 0;
    return deck->size;
}

void ClearDeck(Deck* deck) {
    if (!deck) return;
    deck->size = 0;
}