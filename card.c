#include "cprocessing.h"
#include "card.h"
#include "utils.h"
#include <stdlib.h>
#include "levels.h"
#include "game.h"	
#include <stdio.h>
#include <math.h> // <-- FIX: Added for round()

// --- MODIFIED: New DrawCard function ---
void DrawCard(Card* hand) {
	// 1. Check type for card background color
	switch (hand->type) {
	case Attack:
		CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
		break;
	case Heal:
		CP_Settings_Fill(CP_Color_Create(80, 172, 85, 255));
		break;
	case Shield:
		CP_Settings_Fill(CP_Color_Create(0, 0, 255, 255));
		break;
	default:
		CP_Settings_Fill(CP_Color_Create(100, 100, 100, 255));
		break;
	}

	// 2. Draw the card rectangle (from center)
	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Graphics_DrawRect(hand->pos.x, hand->pos.y, hand->card_w, hand->card_h);

	// 3. Draw the description text (word-wrapped)
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextSize(16); // Small font size to fit on the card
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP); // Align text to the top-center

	// Calculate the bounding box for the text
	float padding = 10.0f;
	float text_box_width = hand->card_w - (padding * 2);
	// Calculate the top-left X and Y for the text box
	float text_box_x = hand->pos.x - (text_box_width / 2.0f);
	float text_box_y = hand->pos.y - (hand->card_h / 2.0f) + padding; // Start from the top

	// Use DrawTextBox for automatic word wrapping
	CP_Font_DrawTextBox(hand->description, text_box_x, text_box_y, text_box_width);
}
// --- END OF MODIFIED DrawCard ---


// Maybe switch to pointer for select index
// --- FIX: Added 'Card hand[]' to match card.h ---
void SelectCard(int index, int* selected, Card hand[]) {
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

void SetHandPos(Card* hand, int hand_size) {
	float rectdelta = 0; // distance of the x coord of the middle of the first card
	float hand_margin = 20;
	float total_hand_width = HandWidth(hand, hand_size, hand_margin);
	// calculate x coord of start of hand by taking the window width - width of hand - 2 to get starting of the hand coord
	// we add hand[0].card_w to get the middle of first card
	float hand_x = ((CP_System_GetWindowWidth() - total_hand_width) / 2.0f) + (hand[0].card_w / 2);
	float hand_y = 600;

	for (int i = 0; i < hand_size; ++i) {
		if (hand[i].is_discarding) continue; // <-- MODIFIED: Skip cards animating to discard

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
		// special : it can choose itself
		int j = rand() % (i + 1);
		// swap
		Card temp = deck[i];
		deck[i] = deck[j];
		deck[j] = temp;
	}
}


// --- FIX: Changed 'hand' to 'hand_slot' to match card.h ---
void DealFromDeck(Card* deck, Card* hand_slot, int* deck_size, int* hand_size) {
	// wait i shuffle each draw??? why
	// comment out maybe
	if (*deck_size > 1) {
		ShuffleDeck(deck, *deck_size);
	}

	// pass first card in deck to hand
	// --- FIX: Use pointer 'hand_slot' ---
	*hand_slot = deck[0];
	// move rest of elements up one slot
	for (int i = 0; i < *deck_size - 1; ++i) {
		deck[i] = deck[i + 1];
	}

	// decrement deck size and increment hand size
	--(*deck_size);
	++(*hand_size);
}

// --- MODIFIED: This function NOW ONLY handles EFFECTS and returns power ---
int UseCard(Card* hand, int* selected_index, int* hand_size, Player* player_ptr, Enemy* enemy_ptr) {

	int power = (int)round(hand[*selected_index].power); // Get the power value

	// --- ALL DAMAGE/HEAL/SHIELD LOGIC HAS BEEN REMOVED FROM HERE ---
	// --- IT IS NOW HANDLED IN GAME.C ---

	// Handle card *effects*
	switch (hand[*selected_index].effect) {
	case DOT:
		if (enemy_ptr) enemy_ptr->dot_timing = 3;
		break;
	default:
		break;
	}

	return power; // Return the damage/heal/shield amount
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
// can use pointers. or pass to deck randomly using rand()
void RecycleDeck(Card* discard, Card* deck, int* discard_size, int* deck_size) {
	// --- NEW: Get deck's top-left pos from game.c global ---
	// (We know it's 50, 600 from game.c)
	CP_Vector deck_pos_topleft = CP_Vector_Set(50, 600);
	CP_Vector deck_pos_center = CP_Vector_Set(
		deck_pos_topleft.x + CARD_W_INIT / 2.0f,
		deck_pos_topleft.y + CARD_H_INIT / 2.0f
	);

	for (int i = 0; i < *discard_size; ++i) {
		deck[i] = discard[i];
		deck[i].pos = deck_pos_center; // <-- MODIFIED: Use center position

		// --- FIX: Reset card state fully ---
		deck[i].is_discarding = false;
		deck[i].is_animating = false;
		deck[i].target_pos = CP_Vector_Set(0, 0);
		// --- END FIX ---

		++(*deck_size);
	}
	*discard_size = 0;
	ShuffleDeck(deck, *deck_size);
}

//todo: card animation
void AnimateMoveCard(Card* hand) {
	// init dt for frame based animation
	float dt = CP_System_GetDt();
	// speed card move for animation
	float speed = 900;
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

//void LoadCardIcon(void) {
//	card_type_icon[Attack] = CP_Image_Load("Assets/sword.png");
//	card_type_icon[Heal] = CP_Image_Load("Assets/suit_hearts.png");
//
//	card_effect_icon[DOT] = CP_Image_Load("Assets/fire.png");
//	card_effect_icon[Draw] = CP_Image_Load("Assets/card_add.png");
//}