#include "card.h"
#include "utils.h"
#include <stdlib.h> // For rand()
#include "levels.h" // For Enemy struct
#include "game.h"	// For Player struct and globals
#include <stdio.h>  // For snprintf()

// --- CARD DRAWING ---

void DrawCard(Card* hand, CP_Image* type_icons, CP_Image* effect_icons) {
	// 1. Draw Card Background
	switch (hand->type) {
	case Attack:
		CP_Settings_Fill(CP_Color_Create(200, 50, 50, 255)); // Dark Red
		break;
	case Heal:
		CP_Settings_Fill(CP_Color_Create(80, 172, 85, 255)); // Green
		break;
	default:
		CP_Settings_Fill(CP_Color_Create(100, 100, 100, 255));
		break;
	}
	// set rect to draw from center
	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Settings_Stroke(CP_Color_Create(255, 255, 255, 255)); // White border
	CP_Settings_StrokeWeight(2.0f);
	CP_Graphics_DrawRect(hand->pos.x, hand->pos.y, hand->card_w, hand->card_h);
	CP_Settings_NoStroke();

	// 2. Draw Card Title
	CP_Font_Set(game_font);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextSize(18.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);
	CP_Font_DrawText(hand->title, hand->pos.x, hand->pos.y - (hand->card_h / 2.0f) + 10.0f);

	// 3. Draw Icons
	// Type Icon (e.g., Sword)
	if (type_icons[hand->type]) {
		CP_Image_Draw(type_icons[hand->type], hand->pos.x - (hand->card_w / 2.0f) + 20.0f, hand->pos.y - 15.0f, 25.0f, 25.0f, 255);
	}
	// Effect Icon (e.g., Fire)
	if (hand->effect != None && effect_icons[hand->effect]) {
		CP_Image_Draw(effect_icons[hand->effect], hand->pos.x - (hand->card_w / 2.0f) + 20.0f, hand->pos.y + 30.0f, 25.0f, 25.0f, 255);
	}

	// 4. Draw Dynamic Description
	char dynamic_description[128];
	int final_power = (int)(hand->power * g_card_multiplier);
	snprintf(dynamic_description, 128, hand->description_template, final_power);

	CP_Settings_Fill(CP_Color_Create(230, 230, 230, 255)); // Light grey text
	CP_Settings_TextSize(14.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_BOTTOM);
	CP_Font_DrawTextBox(dynamic_description, hand->pos.x - (hand->card_w / 2.0f) + 5.0f, hand->pos.y + 10.0f, hand->card_w - 10.0f);
}

// --- CARD ACTIONS ---

void UseCard(Card* hand, int* selected_index, int* hand_size, Player* player_ptr, Enemy* enemy_ptr, Card* deck, int* deck_size, bool did_miss) {

	int card_index = *selected_index;

	// Get the base power from the card
	int base_power = (int)hand[card_index].power;
	// Calculate final power with multiplier
	int final_power = (int)(base_power * g_card_multiplier);

	// --- 1. Main Card Type Logic ---
	if (hand[card_index].type == Attack) {
		if (enemy_ptr->alive && !did_miss) {
			enemy_ptr->health -= final_power;

			// Lifesteal Logic
			if (g_player_has_lifesteal) {
				player_ptr->health += final_power;
				if (player_ptr->health > player_ptr->max_health) {
					player_ptr->health = player_ptr->max_health;
				}
			}
		}
		// (Health check is now in game.c after this function returns)
	}
	else if (hand[card_index].type == Heal) {
		player_ptr->health += final_power;
		// Clamp health to max_health
		if (player_ptr->health > player_ptr->max_health) {
			player_ptr->health = player_ptr->max_health;
		}
	}

	// --- 2. Card Effect Logic ---
	// (This runs in addition to the main type logic)
	switch (hand[card_index].effect) {

	case Draw:
		// Draw 1 card from the deck
		if (*deck_size > 0) {
			DealFromDeck(deck, hand, deck_size, hand_size);
			SetHandPos(hand, *hand_size);
		}
		break;

	case DOT:
		// Apply 3 turns of DOT to the enemy
		if (enemy_ptr->alive && !did_miss) {
			enemy_ptr->dot_timing = 3;
		}
		break;

	case None:
	default:
		// Do no additional effect
		break;
	}
}


// --- DECK & HAND MANAGEMENT ---

void SelectCard(int index, int* selected) {
	// if selected card is selected again, turn select flag to point to no card
	if (*selected == index) {
		*selected = -1;
		return;
	}
	// else change index
	*selected = index;
}

// calculate hand width for hand position
float HandWidth(Card hand[], int size, float margin) {
	float width = 0;
	//loop througn the hand and sum up card with with margin
	for (int i = 0; i < size; ++i) {
		width += (i == size - 1) ? hand[i].card_w : hand[i].card_w + margin; // if last card just add card width to the sum
	}
	return width;
}

void SetHandPos(Card* hand, int hand_size) {
	float rectdelta = 0; // distance of the x coord of the middle of the first card
	float hand_margin = 20.0f;
	float total_hand_width = HandWidth(hand, hand_size, hand_margin);
	float hand_x = ((CP_System_GetWindowWidth() - total_hand_width) / 2.0f) + (hand[0].card_w / 2.0f);
	float hand_y = 600.0f;

	for (int i = 0; i < hand_size; ++i) {
		hand[i].target_pos = CP_Vector_Set(hand_x + rectdelta, hand_y);
		hand[i].is_animating = true; // enable animation for the card to move to target pos
		rectdelta += hand_margin + hand[i].card_w; // set new distance from middle of first card
	}
}

void ShuffleDeck(Card* deck, int deck_size) {
	// loop through deck from the back
	for (int i = deck_size - 1; i > 0; i--) {
		// get swap index.
		// get value [0, i]. so swapped indexes does not get swapped again
		int j = rand() % (i + 1);
		// swap
		Card temp = deck[i];
		deck[i] = deck[j];
		deck[j] = temp;
	}
}


void DealFromDeck(Card* deck, Card* hand, int* deck_size, int* hand_size) {
	if (*deck_size > 1) {
		ShuffleDeck(deck, *deck_size);
	}

	// pass first card in deck to hand
	hand[*hand_size] = deck[0];

	// move rest of elements up one slot
	for (int i = 0; i < *deck_size - 1; ++i) {
		deck[i] = deck[i + 1];
	}

	// decrement deck size and increment hand size
	--(*deck_size);
	++(*hand_size);
}

void DiscardCard(Card* hand, int* selected_index, int* hand_size, Card* discard, int* discard_size) {
	// add selected card to discard pile
	discard[*discard_size] = hand[*selected_index];
	// increment discard size
	++(*discard_size);

	// remove card from hand array
	for (int i = *selected_index; i < *hand_size - 1; ++i) {
		// move card up to replace the used card
		hand[i] = hand[i + 1];
	}

	// decrement hand size
	--(*hand_size);
	// reset select index
	*selected_index = -1;
}

// assume deck size 0
void RecycleDeck(Card* discard, Card* deck, int* discard_size, int* deck_size) {
	for (int i = 0; i < *discard_size; ++i) {
		deck[i] = discard[i];
		deck[i].pos = CP_Vector_Set(50, 600); // Reset position to the deck
		++(*deck_size);
	}
	*discard_size = 0;
	ShuffleDeck(deck, *deck_size);
}

void AnimateMoveCard(Card* hand) {
	// init dt for frame based animation
	float dt = CP_System_GetDt();
	// speed card move for animation
	float speed = 900.0f;
	// get the direction vector in terms of how far the card is from the target position
	CP_Vector direction = CP_Vector_Subtract(hand->target_pos, hand->pos);
	// get euclidean distance of card from pos through the direction vector
	float distance = CP_Vector_Length(direction);

	// if very close to  target pos, snap to target pos
	if (distance < 1.0f) {
		hand->pos = hand->target_pos;
		// set animating flag to no
		hand->is_animating = false;
		return;
	}

	// init movement vector to the normalized direction vector  scaled to speed * dt to move with frame rate
	CP_Vector move = CP_Vector_Scale(CP_Vector_Normalize(direction), speed * dt);

	// if movement vector will move pass the distance snap to target pos
	if (CP_Vector_Length(move) > distance) {
		hand->pos = hand->target_pos;
	}
	else { // move the card
		hand->pos = CP_Vector_Add(hand->pos, move);
	}
}