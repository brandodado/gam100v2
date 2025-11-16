#include "deck.h"
#include "cprocessing.h"
#include <stdlib.h>
#include <string.h>

void InitDeck(Deck* deck) {
    if (!deck) return;

    deck->size = 0;
    deck->capacity = MAX_DECK_SIZE;

    // Initialize with starter cards
    CP_Vector deck_pos_topleft = CP_Vector_Set(50, 600); // This is the top-left corner
    CP_Vector deck_pos_center = CP_Vector_Set(
        deck_pos_topleft.x + CARD_W_INIT / 2.0f,
        deck_pos_topleft.y + CARD_H_INIT / 2.0f
    );

    // Create starter cards
    Card attack = {
        deck_pos_center,
        CP_Vector_Set(0, 0),
        Attack,
        None,
        7, // <-- MODIFIED: Base damage is now 7
        "Deals 7 damage.", // <-- MODIFIED: Updated description
        CARD_W_INIT,
        CARD_H_INIT,
        false,
        false // (is_discarding)
    };

    Card heal = {
        deck_pos_center,
        CP_Vector_Set(0, 0),
        Heal,
        None,
        7,
        "Heals 7 HP.",
        CARD_W_INIT,
        CARD_H_INIT,
        false,
        false // (is_discarding)
    };

    // --- NEW: Create Shield Card ---
    Card shield = {
        deck_pos_center,
        CP_Vector_Set(0, 0),
        Shield,
        None,
        5,
        "Gain 5 Shield.",
        CARD_W_INIT,
        CARD_H_INIT,
        false,
        false // (is_discarding)
    };

    // Add starter cards
    for (int i = 0; i < 6; i++) {
        AddCardToDeck(deck, attack);
    }
    for (int i = 0; i < 4; i++) {
        AddCardToDeck(deck, heal);
    }
    // --- NEW: Add Shield Cards ---
    for (int i = 0; i < 4; i++) {
        AddCardToDeck(deck, shield);
    }
}

bool AddCardToDeck(Deck* deck, Card card) {
    if (!deck || deck->size >= deck->capacity) {
        return false;
    }

    deck->cards[deck->size] = card;
    deck->size++;
    return true;
}

bool RemoveCardFromDeck(Deck* deck, int index) {
    if (!deck || index < 0 || index >= deck->size) {
        return false;
    }

    // Shift all cards after the removed card
    for (int i = index; i < deck->size - 1; i++) {
        deck->cards[i] = deck->cards[i + 1];
    }

    deck->size--;
    return true;
}

Card GetDeckCard(Deck* deck, int index) {
    if (!deck || index < 0 || index >= deck->size) {
        // Return a default card if invalid
        Card default_card = {
            CP_Vector_Set(0, 0),
            CP_Vector_Set(0, 0),
            Attack,
            None,
            0,
            "Invalid",
            CARD_W_INIT,
            CARD_H_INIT,
            false,
            false // (is_discarding)
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