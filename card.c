#include "cprocessing.h"
#include "card.h"
#include "utils.h"
#include "levels.h"
#include "game.h"	
#include <stdio.h>
#include <math.h> 

void DrawCard(Card* hand) {
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

	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Graphics_DrawRect(hand->pos.x, hand->pos.y, hand->card_w, hand->card_h);

	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextSize(16);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);

	float padding = 10.0f;
	float text_box_width = hand->card_w - (padding * 2);
	float text_box_x = hand->pos.x - (text_box_width / 2.0f);
	float text_box_y = hand->pos.y - (hand->card_h / 2.0f) + padding;

	CP_Font_DrawTextBox(hand->description, text_box_x, text_box_y, text_box_width);
}

void SelectCard(int index, int* selected, Card hand[]) {
	if (*selected == index) {
		*selected = -1;
		return;
	}
	*selected = index;
}

float HandWidth(Card hand[], int size, float margin) {
	float width = 0;
	for (int i = 0; i < size; ++i) {
		width += (i == size - 1) ? hand[i].card_w : hand[i].card_w + margin;
	}
	return width;
}

void SetHandPos(Card* hand, int hand_size) {
	float rectdelta = 0;
	float hand_margin = 20;
	float total_hand_width = HandWidth(hand, hand_size, hand_margin);
	float hand_x = ((CP_System_GetWindowWidth() - total_hand_width) / 2.0f) + (hand[0].card_w / 2);
	float hand_y = 600;

	for (int i = 0; i < hand_size; ++i) {
		if (hand[i].is_discarding) continue;

		hand[i].target_pos = CP_Vector_Set(hand_x + rectdelta, hand_y);
		hand[i].is_animating = true;
		rectdelta += hand_margin + hand[i].card_w;
	}
}

void ShuffleDeck(Card* deck, int deck_size) {
	for (int i = deck_size - 1; i > 0; i--) {
		int j = CP_Random_RangeInt(0, i);
		Card temp = deck[i];
		deck[i] = deck[j];
		deck[j] = temp;
	}
}

void DealFromDeck(Card* deck, Card* hand_slot, int* deck_size, int* hand_size) {
	if (*deck_size > 1) {
		ShuffleDeck(deck, *deck_size);
	}
	else if (*deck_size <= 0) {
		return;
	}

	*hand_slot = deck[0];
	for (int i = 0; i < *deck_size - 1; ++i) {
		deck[i] = deck[i + 1];
	}

	--(*deck_size);
	++(*hand_size);
}

// --- CRITICAL FIX: selected_index is now an int, not an int* ---
int UseCard(Card* hand, int selected_index, int* hand_size, Player* player_ptr, Enemy* enemy_ptr) {
	int power = (int)round(hand[selected_index].power);

	switch (hand[selected_index].effect) {
	case DOT:
		if (enemy_ptr) enemy_ptr->dot_timing = 3;
		break;
	default:
		break;
	}
	return power;
}

// --- CRITICAL FIX: selected_index is now an int, not an int* ---
void DiscardCard(Card* hand, int selected_index, int* hand_size, Card* discard, int* discard_size) {
	discard[*discard_size] = hand[selected_index];
	++(*discard_size);

	for (int i = selected_index; i < *hand_size - 1; ++i) {
		hand[i] = hand[i + 1];
	}

	--(*hand_size);
}

void RecycleDeck(Card* discard, Card* deck, int* discard_size, int* deck_size) {
	CP_Vector deck_pos_topleft = CP_Vector_Set(50, 600);
	CP_Vector deck_pos_center = CP_Vector_Set(
		deck_pos_topleft.x + CARD_W_INIT / 2.0f,
		deck_pos_topleft.y + CARD_H_INIT / 2.0f
	);

	for (int i = 0; i < *discard_size; ++i) {
		deck[i] = discard[i];
		deck[i].pos = deck_pos_center;

		deck[i].is_discarding = false;
		deck[i].is_animating = false;
		deck[i].target_pos = CP_Vector_Set(0, 0);

		++(*deck_size);
	}
	*discard_size = 0;
	ShuffleDeck(deck, *deck_size);
}

void AnimateMoveCard(Card* card_to_animate) {
	float dt = CP_System_GetDt();
	float speed = 900;
	CP_Vector direction = CP_Vector_Subtract(card_to_animate->target_pos, card_to_animate->pos);
	float distance = CP_Vector_Length(direction);

	if (distance < 1.0f) {
		card_to_animate->pos = card_to_animate->target_pos;
		card_to_animate->is_animating = false;
		return;
	}

	CP_Vector move = CP_Vector_Scale(CP_Vector_Normalize(direction), speed * dt);

	if (CP_Vector_Length(move) > distance) {
		card_to_animate->pos = card_to_animate->target_pos;
	}
	else {
		card_to_animate->pos = CP_Vector_Add(card_to_animate->pos, move);
	}
}