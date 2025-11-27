#include "cprocessing.h"
#include "card.h"
#include "utils.h"
#include "levels.h"
#include "game.h"	
#include <stdio.h>
#include <math.h>
#include <string.h>


Card catalogue[50];
int catalogue_size;

#define RECYCLE_ANIMATE_CARDS 3
static Card recycle_card[RECYCLE_ANIMATE_CARDS];
bool is_recycling = false;

// Static storage for icon assets
static CP_Image icon_attack = NULL;
static CP_Image icon_heal = NULL;
static CP_Image icon_shield = NULL;

// deck.c
// Matching scale factor again
#define CARD_SCALE 1.5f

// This function builds the initial starting deck when a new game begins
void InitDeck(Deck* deck) {
	if (!deck) return;

	deck->size = 0;
	deck->capacity = MAX_DECK_SIZE;

	// Position for spawn
	CP_Vector deck_pos_topleft = CP_Vector_Set(50, 550);
	CP_Vector deck_pos_center = CP_Vector_Set(
		deck_pos_topleft.x + (CARD_W_INIT * CARD_SCALE) / 2.0f,
		deck_pos_topleft.y + (CARD_H_INIT * CARD_SCALE) / 2.0f
	);

	float final_w = CARD_W_INIT * CARD_SCALE;
	float final_h = CARD_H_INIT * CARD_SCALE;

	// Find basic cards from catalogue (power 7 attack, 7 heal, 5 shield, all None effect)
	Card* basic_attack = NULL;
	Card* basic_heal = NULL;
	Card* basic_shield = NULL;

	for (int i = 0; i < catalogue_size; i++) {
		if (catalogue[i].type == Attack && catalogue[i].effect == None && catalogue[i].power == 7) {
			basic_attack = &catalogue[i];
		}
		if (catalogue[i].type == Heal && catalogue[i].effect == None && catalogue[i].power == 7) {
			basic_heal = &catalogue[i];
		}
		if (catalogue[i].type == Shield && catalogue[i].effect == None && catalogue[i].power == 5) {
			basic_shield = &catalogue[i];
		}
	}

	// Fallback if catalogue didn't load
	Card attack = {
		deck_pos_center, CP_Vector_Set(0, 0), Attack, None, 7,
		"Deal 7 Damage.", final_w, final_h, false, false
	};
	Card heal = {
		deck_pos_center, CP_Vector_Set(0, 0), Heal, None, 7,
		"Heal 7 HP.", final_w, final_h, false, false
	};
	Card shield = {
		deck_pos_center, CP_Vector_Set(0, 0), Shield, None, 5,
		"Gain 5 Shield.", final_w, final_h, false, false
	};

	// Use catalogue cards if available
	if (basic_attack) attack = *basic_attack;
	if (basic_heal) heal = *basic_heal;
	if (basic_shield) shield = *basic_shield;

	// Set proper dimensions
	attack.card_w = heal.card_w = shield.card_w = final_w;
	attack.card_h = heal.card_h = shield.card_h = final_h;
	attack.pos = heal.pos = shield.pos = deck_pos_center;

	// Add starter cards
	for (int i = 0; i < 5; i++) AddCardToDeck(deck, attack);
	for (int i = 0; i < 5; i++) AddCardToDeck(deck, heal);
	for (int i = 0; i < 5; i++) AddCardToDeck(deck, shield);
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

// --- MODIFIED: New DrawCard function ---
void DrawCard(Card* hand) {
	// --- 1. Lazy Load Assets ---
	// Only load the images if we haven't done so already
	if (icon_attack == NULL) icon_attack = CP_Image_Load("Assets/icon_sword.png");
	if (icon_heal == NULL)   icon_heal = CP_Image_Load("Assets/icon_heart.png");
	if (icon_shield == NULL) icon_shield = CP_Image_Load("Assets/icon_shield.png");

	// --- 2. Determine Color & Icon based on Type ---
	CP_Color type_color;
	CP_Image current_icon = NULL;

	switch (hand->type) {
	case Attack:
		type_color = CP_Color_Create(255, 0, 0, 255); // Red
		current_icon = icon_attack;
		break;
	case Heal:
		type_color = CP_Color_Create(80, 172, 85, 255); // Green
		current_icon = icon_heal;
		break;
	case Shield:
		type_color = CP_Color_Create(0, 0, 255, 255); // Blue
		current_icon = icon_shield;
		break;
	default:
		type_color = CP_Color_Create(100, 100, 100, 255); // Gray fallback
		break;
	}

	// --- 3. Draw Card Body ---
	CP_Settings_Fill(CP_Color_Create(50, 50, 50, 255)); // Dark Gray Background

	CP_Settings_Stroke(type_color); // Colored border based on type
	CP_Settings_StrokeWeight(3.0f);

	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Graphics_DrawRect(hand->pos.x, hand->pos.y, hand->card_w, hand->card_h);

	// Dimensions for internal layout
	float padding = 8.0f;
	float content_w = hand->card_w - (padding * 2);
	float total_h = hand->card_h - (padding * 2);

	// --- 4. Top Image Area ---
	// Uses top 50% of the inner area for the icon
	float image_h = total_h * 0.50f;

	float image_center_y = (hand->pos.y - (hand->card_h / 2.0f)) + padding + (image_h / 2.0f);

	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_StrokeWeight(1.0f);
	CP_Settings_Stroke(CP_Color_Create(0, 0, 0, 255));
	CP_Graphics_DrawRect(hand->pos.x, image_center_y, content_w, image_h);

	// --- 5. Draw the Icon ---
	if (current_icon != NULL) {
		float icon_size = image_h * 0.8f; // Scale icon to fit box
		CP_Image_Draw(current_icon, hand->pos.x, image_center_y, icon_size, icon_size, 255);
	}

	// --- 6. Description Text Box (Bottom) ---
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextSize(13.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);

	CP_Settings_StrokeWeight(0.0f); // No outline for text

	float text_y_start = image_center_y + (image_h / 2.0f) + padding;

	CP_Font_DrawTextBox(hand->description, hand->pos.x - (content_w / 2.0f), text_y_start, content_w);

	// Reset stroke for future drawing
	CP_Settings_StrokeWeight(1.0f);
	CP_Settings_Stroke(CP_Color_Create(0, 0, 0, 255));
}



void SelectCard(int index, int* selected) {
	// if selected card is selected again, turn select flag to point to no card
	if (*selected == index) {
		*selected = -1;
		return;
	}
	// else change index
	// change selected index
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

// edited to fit all cards regardless of size

float CalculateCardScale(int hand_size) {
	float window_width = CP_System_GetWindowWidth();
	float available_width = window_width - 200.0f; // Leave space for deck/discard piles
	float hand_margin = 20.0f;
	float base_card_width = CARD_W_INIT * CARD_SCALE;

	// Calculate total width needed at full scale
	float total_width = (hand_size * base_card_width) + ((hand_size - 1) * hand_margin);

	// If hand fits, use full scale
	if (total_width <= available_width) {
		return CARD_SCALE;
	}

	// Otherwise, scale down proportionally
	float max_card_width = (available_width - ((hand_size - 1) * hand_margin)) / hand_size;
	return max_card_width / CARD_W_INIT;
}

void SetHandPos(Card* hand, int hand_size) {
	if (hand_size == 0) return;

	float hand_margin = 20;
	float window_width = CP_System_GetWindowWidth();
	float available_width = window_width - 200.0f; // Reserve space for piles

	// Calculate scale factor
	float scale = CalculateCardScale(hand_size);

	// Apply scale to all cards
	for (int i = 0; i < hand_size; ++i) {
		hand[i].card_w = CARD_W_INIT * scale;
		hand[i].card_h = CARD_H_INIT * scale;
	}

	// Calculate positions
	float total_hand_width = HandWidth(hand, hand_size, hand_margin);
	float hand_x = ((window_width - total_hand_width) / 2.0f) + (hand[0].card_w / 2);
	float hand_y = 600;

	float rectdelta = 0;
	for (int i = 0; i < hand_size; ++i) {
		if (hand[i].is_discarding) continue;

		hand[i].target_pos = CP_Vector_Set(hand_x + rectdelta, hand_y);
		hand[i].is_animating = true;
		rectdelta += hand_margin + hand[i].card_w;
	}
}

// change to CProcessing_Random
void ShuffleDeck(Deck* deck) {

	// loop through deck from the back
	for (int i = deck->size - 1; i > 0; i--) {
		// get swap index.
		// get value [0, i]. so swapped indexes does not get swapped again
		// special : it can choose itself
		int j = CP_Random_RangeInt(0, i);
		// swap
		Card temp = deck->cards[i];
		deck->cards[i] = deck->cards[j];
		deck->cards[j] = temp;
	}
}

// look into changes
// --- FIX: Changed 'hand' to 'hand_slot' to match card.h ---
void DealFromDeck(Deck* deck, Card* hand_slot, int* hand_size) {
	if (deck->size <= 0) return; // Safety check

	// Copy card from deck to hand
	*hand_slot = deck->cards[0];

	// Set the starting position to deck center for animation
	CP_Vector deck_pos_center = CP_Vector_Set(
		50 + (CARD_W_INIT * 1.5f) / 2.0f,
		550 + (CARD_H_INIT * 1.5f) / 2.0f
	);
	hand_slot->pos = deck_pos_center;
	hand_slot->is_animating = true;

	// Shift remaining deck cards
	for (int i = 0; i < deck->size - 1; ++i) {
		deck->cards[i] = deck->cards[i + 1];
	}

	// Update counters
	--(deck->size);
	++(*hand_size);
}

// Use card and return its power value
int UseCard(Card* hand, int* selected_index, int* hand_size, Player* player_ptr, Enemy* enemy_ptr) {
	if (!hand || !selected_index || *selected_index < 0 || *selected_index >= *hand_size) {
		return 0;
	}

	int power = hand[*selected_index].power;

	// Handle card effects (if needed in the future)
	switch (hand[*selected_index].effect) {
	case Draw:
		// Could implement draw effect here if needed
		break;
	default:
		break;
	}

	return power; // Return the power value
}
//void DiscardCard(Card* hand, int* selected_index, int* hand_size, Card* discard, int* discard_size) {
//	// add selected card to discard pile
//	discard[*discard_size] = hand[*selected_index];
//	// increment discard size
//	++(*discard_size);
//
//	// remove card from hand array
//	for (int i = *selected_index; i < *hand_size - 1; ++i) {
//		// move card up to replace the used card
//		hand[i] = hand[i + 1];
//	}
//
//
//	// decrement hand size
//	--(*hand_size);
//	// reset select index
//	*selected_index = -1;
//}

// assume deck size 0
// can use pointers. or pass to deck randomly using rand()
void RecycleDeck(Card* discard, Deck* deck, int* discard_size) {
	// --- NEW: Get deck's top-left pos from game.c global ---
	// (We know it's 50, 600 from game.c)
	CP_Vector deck_pos_topleft = CP_Vector_Set(50, 600);
	CP_Vector deck_pos_center = CP_Vector_Set(
		deck_pos_topleft.x + CARD_W_INIT / 2.0f,
		deck_pos_topleft.y + CARD_H_INIT / 2.0f
	);

	for (int i = 0; i < *discard_size; ++i) {
		deck->cards[i] = discard[i];
		deck->cards[i].pos = deck_pos_center; // <-- MODIFIED: Use center position

		// --- FIX: Reset card state fully ---
		deck->cards[i].is_discarding = false;
		deck->cards[i].is_animating = false;
		deck->cards[i].target_pos = CP_Vector_Set(0, 0);
		// --- END FIX ---

		++(deck->size);
	}
	*discard_size = 0;
	ShuffleDeck(deck);
}

//todo: card animation
void AnimateMoveCard(Card* hand, float speed) {
	// init dt for frame based animation
	float dt = CP_System_GetDt();
	// speed card move for animation
	// get the direction vector in terms of how far the card is from the target position
	CP_Vector direction = CP_Vector_Subtract(hand->target_pos, hand->pos);
	// get euclidean distance of card from pos through the direction vector
	float distance = CP_Vector_Length(direction);

	// if very close to  target pos, snap to target pos
	if (distance < 1.0f) {
		hand->pos = hand->target_pos;
		// set animating flag to false
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

//see if i can still put icons somewhere
//void LoadCardIcon(void) {
//	card_type_icon[Attack] = CP_Image_Load("Assets/sword.png");
//	card_type_icon[Heal] = CP_Image_Load("Assets/suit_hearts.png");
//
//	card_effect_icon[DOT] = CP_Image_Load("Assets/fire.png");
//	card_effect_icon[Draw] = CP_Image_Load("Assets/card_add.png");
//}

CardEffect StringToEffect(const char* s) {
	//if (strcmp(s, "DOT") == 0) return DOT;
	if (strcmp(s, "Draw") == 0) return Draw;
	if (strcmp(s, "SHIELD_BASH") == 0) return SHIELD_BASH;
	if (strcmp(s, "CLEAVE") == 0) return CLEAVE;
	if (strcmp(s, "DIVINE_STRIKE_EFFECT") == 0) return DIVINE_STRIKE_EFFECT;
	return None;
}


CardType StringToType(const char* s) {
	if (strcmp(s, "Attack") == 0) return Attack;
	if (strcmp(s, "Heal") == 0) return Heal;
	if (strcmp(s, "Shield") == 0) return Shield;
	return Attack;
}

//add loading of cards from txt file
// return number of card loaded
int LoadCatalogue(const char* fcat, Card* cat_arr, int max_size) {
	FILE* catalogue = fopen(fcat, "r");
	if (!catalogue) {
		// display error
		return 0;
	}

	char string[255];
	char type[30];
	char effect[30];
	char desc[200];

	int count = 0;

	while (fgets(string, 255, catalogue) != NULL && count < max_size) {
		sscanf_s(string, "%29[^,],%29[^,],%d,%199[^\n]",
			type, sizeof(type),
			effect, sizeof(effect),
			&cat_arr[count].power,
			desc, sizeof(desc));
		cat_arr[count].type = StringToType(type);
		cat_arr[count].effect = StringToEffect(effect);
		strcpy_s(cat_arr[count].description, sizeof(cat_arr[count].description), desc);
		cat_arr[count].pos = cat_arr[count].target_pos = CP_Vector_Set(0, 0);
		cat_arr[count].is_animating = false;
		cat_arr[count].is_discarding = false;
		cat_arr[count].card_h = CARD_H_INIT;
		cat_arr[count].card_w = CARD_W_INIT;
		count++;
	}

	fclose(catalogue);
	return count;
}