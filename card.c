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

// deck.c handles InitDeck, AddCardToDeck, etc. now.
// Matching scale factor again
#define CARD_SCALE 1.5f

// --- DrawCard function ---
void DrawCard(Card* hand, Player* player) {
    // --- 1. Lazy Load Assets ---
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
    float image_h = total_h * 0.50f;
    float image_center_y = (hand->pos.y - (hand->card_h / 2.0f)) + padding + (image_h / 2.0f);

    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Settings_StrokeWeight(1.0f);
    CP_Settings_Stroke(CP_Color_Create(0, 0, 0, 255));
    CP_Graphics_DrawRect(hand->pos.x, image_center_y, content_w, image_h);

    // --- 5. Draw the Icon ---
    if (current_icon != NULL) {
        float icon_size = image_h * 0.8f;
        CP_Image_Draw(current_icon, hand->pos.x, image_center_y, icon_size, icon_size, 255);
    }

    // --- 6. Description Text Box (Bottom) ---
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Settings_TextSize(13.0f);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);
    CP_Settings_StrokeWeight(0.0f);

    float text_y_start = image_center_y + (image_h / 2.0f) + padding;

    CP_Font_DrawTextBox(hand->description, hand->pos.x - (content_w / 2.0f), text_y_start, content_w);

    // Reset stroke
    CP_Settings_StrokeWeight(1.0f);
    CP_Settings_Stroke(CP_Color_Create(0, 0, 0, 255));
}

void SelectCard(int index, int* selected) {
    if (*selected == index) {
        *selected = -1;
        return;
    }
    *selected = index;
}

// Calculate hand width for hand position
float HandWidth(Card hand[], int size, float margin) {
    float width = 0;
    for (int i = 0; i < size; ++i) {
        width += (i == size - 1) ? hand[i].card_w : hand[i].card_w + margin;
    }
    return width;
}

float CalculateCardScale(int hand_size) {
    float window_width = (float)CP_System_GetWindowWidth();
    float available_width = window_width - 200.0f;
    float hand_margin = 20.0f;
    float base_card_width = CARD_W_INIT * CARD_SCALE;

    float total_width = (hand_size * base_card_width) + ((hand_size - 1) * hand_margin);

    if (total_width <= available_width) {
        return CARD_SCALE;
    }

    float max_card_width = (available_width - ((hand_size - 1) * hand_margin)) / hand_size;
    return max_card_width / CARD_W_INIT;
}

void SetHandPos(Card* hand, int hand_size) {
    if (hand_size == 0) return;

    float hand_margin = 20.0f;
    float window_width = (float)CP_System_GetWindowWidth();
    // float available_width = window_width - 200.0f; // Unused variable

    float scale = CalculateCardScale(hand_size);

    for (int i = 0; i < hand_size; ++i) {
        hand[i].card_w = CARD_W_INIT * scale;
        hand[i].card_h = CARD_H_INIT * scale;
    }

    float total_hand_width = HandWidth(hand, hand_size, hand_margin);
    float hand_x = ((window_width - total_hand_width) / 2.0f) + (hand[0].card_w / 2);
    float hand_y = 600.0f;

    float rectdelta = 0;
    for (int i = 0; i < hand_size; ++i) {
        if (hand[i].is_discarding) continue;

        hand[i].target_pos = CP_Vector_Set(hand_x + rectdelta, hand_y);
        hand[i].is_animating = true;
        rectdelta += hand_margin + hand[i].card_w;
    }
}

void ShuffleDeck(Deck* deck) {
    for (int i = deck->size - 1; i > 0; i--) {
        int j = CP_Random_RangeInt(0, i);
        Card temp = deck->cards[i];
        deck->cards[i] = deck->cards[j];
        deck->cards[j] = temp;
    }
}

void DealFromDeck(Deck* deck, Card* hand_slot, int* hand_size) {
    if (deck->size <= 0) return;

    *hand_slot = deck->cards[0];

    CP_Vector deck_pos_center = CP_Vector_Set(
        50.0f + (CARD_W_INIT * 1.5f) / 2.0f,
        550.0f + (CARD_H_INIT * 1.5f) / 2.0f
    );
    hand_slot->pos = deck_pos_center;
    hand_slot->is_animating = true;

    for (int i = 0; i < deck->size - 1; ++i) {
        deck->cards[i] = deck->cards[i + 1];
    }

    --(deck->size);
    ++(*hand_size);
}

int UseCard(Card* hand, int* selected_index, int* hand_size, Player* player_ptr, Enemy* enemy_ptr) {
    if (!hand || !selected_index || *selected_index < 0 || *selected_index >= *hand_size) {
        return 0;
    }
    int power = hand[*selected_index].power;
    // Effects handled in game.c logic mostly
    return power;
}

void RecycleDeck(Card* discard, Deck* deck, int* discard_size) {
    CP_Vector deck_pos_topleft = CP_Vector_Set(50.0f, 600.0f);
    CP_Vector deck_pos_center = CP_Vector_Set(
        deck_pos_topleft.x + CARD_W_INIT / 2.0f,
        deck_pos_topleft.y + CARD_H_INIT / 2.0f
    );

    for (int i = 0; i < *discard_size; ++i) {
        deck->cards[i] = discard[i];
        deck->cards[i].pos = deck_pos_center;

        deck->cards[i].is_discarding = false;
        deck->cards[i].is_animating = false;
        deck->cards[i].target_pos = CP_Vector_Set(0.0f, 0.0f);

        ++(deck->size);
    }
    *discard_size = 0;
    ShuffleDeck(deck);
}

void AnimateMoveCard(Card* hand, float speed) {
    float dt = CP_System_GetDt();
    CP_Vector direction = CP_Vector_Subtract(hand->target_pos, hand->pos);
    float distance = CP_Vector_Length(direction);

    if (distance < 1.0f) {
        hand->pos = hand->target_pos;
        hand->is_animating = false;
        return;
    }

    CP_Vector move = CP_Vector_Scale(CP_Vector_Normalize(direction), speed * dt);

    if (CP_Vector_Length(move) > distance) {
        hand->pos = hand->target_pos;
    }
    else {
        hand->pos = CP_Vector_Add(hand->pos, move);
    }
}

CardEffect StringToEffect(const char* s) {
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

int LoadCatalogue(const char* fcat, Card* cat_arr, int max_size) {
    FILE* catalogue_file = fopen(fcat, "r");
    if (!catalogue_file) {
        return 0;
    }

    char string[255];
    char type[30];
    char effect[30];
    char desc[200];

    int count = 0;

    while (fgets(string, 255, catalogue_file) != NULL && count < max_size) {
        sscanf_s(string, "%29[^,],%29[^,],%d,%199[^\n]",
            type, (unsigned int)sizeof(type),
            effect, (unsigned int)sizeof(effect),
            &cat_arr[count].power,
            desc, (unsigned int)sizeof(desc));

        cat_arr[count].type = StringToType(type);
        cat_arr[count].effect = StringToEffect(effect);
        strcpy_s(cat_arr[count].description, sizeof(cat_arr[count].description), desc);
        cat_arr[count].pos = cat_arr[count].target_pos = CP_Vector_Set(0.0f, 0.0f);
        cat_arr[count].is_animating = false;
        cat_arr[count].is_discarding = false;
        cat_arr[count].card_h = CARD_H_INIT;
        cat_arr[count].card_w = CARD_W_INIT;
        count++;
    }

    fclose(catalogue_file);
    return count;
}