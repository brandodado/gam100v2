#include "cprocessing.h"
#include "card.h"
#include "utils.h"
#include "levels.h"
#include "game.h"	
#include <stdio.h>
#include <math.h>
#include <string.h>

// for the cataloguing of cards
Card catalogue[50];
int catalogue_size;


// Static storage for icon assets
static CP_Image icon_attack = NULL;
static CP_Image icon_heal = NULL;
static CP_Image icon_shield = NULL;

// deck.c handles InitDeck, AddCardToDeck, etc. now.
// Matching scale factor again
#define CARD_SCALE 1.5f

// --- DrawCard function ---
void DrawCard(Card* hand) {
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
    // if current card index is selected index
    if (*selected == index) {
        // deselect all cards
        *selected = -1;
    }
    else {
        // current index is selected
        *selected = index;
    }
   
}

// Calculate hand width for hand position
float HandWidth(Card hand[], int size, float margin) {
    // sum variable
    float width = 0;
    for (int i = 0; i < size; ++i) {
        // if last we just add card w do not need to add margin between cards
        width += (i == size - 1) ? hand[i].card_w : hand[i].card_w + margin;
    }
    return width;
}

float CalculateCardScale(int hand_size) {
    float window_width = (float)CP_System_GetWindowWidth();
    float available_width = window_width - 200.0f;
    float hand_margin = 20.0f; // space between cards in hand
    
    float base_card_width = CARD_W_INIT * CARD_SCALE;

    float total_width = (hand_size * base_card_width) + ((hand_size - 1) * hand_margin);

    if (total_width <= available_width) {
        return CARD_SCALE;
    }

    float max_card_width = (available_width - ((hand_size - 1) * hand_margin)) / hand_size;
    return max_card_width / CARD_W_INIT;
}

void SetHandPos(Card* hand, int hand_size) {
    // if no card in hand do not proceed.
    if (hand_size == 0) return;

    float hand_margin = 20.0f;
    float window_width = (float)CP_System_GetWindowWidth();

    float scale = CalculateCardScale(hand_size);

    for (int i = 0; i < hand_size; ++i) {
        hand[i].card_w = CARD_W_INIT * scale;
        hand[i].card_h = CARD_H_INIT * scale;
    }

    // get hand width of all cards in hand + margin
    float total_hand_width = HandWidth(hand, hand_size, hand_margin);
    // set starting hand x based on card handwidth and half card width as we draw from center
    float hand_x = ((window_width - total_hand_width) / 2.0f) + (hand[0].card_w / 2);
    // we set hand y at static 600 pixels to be closer to the bottom of the screen
    float hand_y = 600.0f;

    // variable to collect distance from starting x position to get values to draw cards after first index
    float rectdelta = 0;
    for (int i = 0; i < hand_size; ++i) {
        // flag to not draw discarding card
        if (hand[i].is_discarding) continue;

        // set to target pos so they can animate there
        hand[i].target_pos = CP_Vector_Set(hand_x + rectdelta, hand_y);
        hand[i].is_animating = true;
        // increase the distance from the first card
        rectdelta += hand_margin + hand[i].card_w;
    }
}

void ShuffleDeck(Deck* deck) {
    // shuffles from the back
    for (int i = deck->size - 1; i > 0; i--) {
        // get an index from the cards before current index.
        // will not touch already shuffled cards and cards will not stay in space
        int j = CP_Random_RangeInt(0, i);
        // swap
        Card temp = deck->cards[i];
        deck->cards[i] = deck->cards[j];
        deck->cards[j] = temp;
    }
}

void DealFromDeck(Deck* deck, Card* hand_slot, int* hand_size) {
    // if deck is empty terminate the fucntion call
    if (deck->size <= 0) return;

    // deal the card to the current hand index through pointer
    *hand_slot = deck->cards[0];

    CP_Vector deck_pos_center = CP_Vector_Set(
        50.0f + (CARD_W_INIT * 1.5f) / 2.0f,
        550.0f + (CARD_H_INIT * 1.5f) / 2.0f
    );
    // set the hand pos to deck so it draws from deck and will move to hand slot position
    hand_slot->pos = deck_pos_center;
    hand_slot->is_animating = true;

    // move cards in deck up a slot for next draw
    for (int i = 0; i < deck->size - 1; ++i) {
        deck->cards[i] = deck->cards[i + 1];
    }

    // update deck and hand size
    --(deck->size);
    ++(*hand_size);
}

void RecycleDeck(Card* discard, Deck* deck, int* discard_size) {
    CP_Vector deck_pos_topleft = CP_Vector_Set(50.0f, 600.0f);
    CP_Vector deck_pos_center = CP_Vector_Set(
        deck_pos_topleft.x + CARD_W_INIT / 2.0f,
        deck_pos_topleft.y + CARD_H_INIT / 2.0f
    );

    for (int i = 0; i < *discard_size; ++i) {
        // pass cards from discard pile to last slot in deck
        deck->cards[deck->size] = discard[i];
        deck->cards[deck->size].pos = deck_pos_center;

        deck->cards[deck->size].is_discarding = false;
        deck->cards[deck->size].is_animating = false;
        deck->cards[deck->size].target_pos = CP_Vector_Set(0.0f, 0.0f);

        // increment deck size
        ++(deck->size);
    }
    // set discard size to zero as there is no more cards
    *discard_size = 0;

    ShuffleDeck(deck);
}

void AnimateMoveCard(Card* hand, float speed) {
    // get dt value to animate according to frame rate
    float dt = CP_System_GetDt();

    // get relative direction to target location and distance to location 
    CP_Vector direction = CP_Vector_Subtract(hand->target_pos, hand->pos);
    float distance = CP_Vector_Length(direction);

    // set move vector to a scaled vector of the unit vector of the direction and the speed scaled by delta time
    CP_Vector move = CP_Vector_Scale(CP_Vector_Normalize(direction), speed * dt);

    // if the movement vector is more than the distance just snap the card to postion
    if (CP_Vector_Length(move) > distance) {
        hand->pos = hand->target_pos;
        hand->is_animating = false;
    }
    else {
        // add movement vector
        hand->pos = CP_Vector_Add(hand->pos, move);
    }
}

// to parse stirng taken from catalogue file into a effect type
CardEffect StringToEffect(const char* s) {
    if (strcmp(s, "Draw") == 0) return Draw;
    if (strcmp(s, "SHIELD_BASH") == 0) return SHIELD_BASH;
    if (strcmp(s, "CLEAVE") == 0) return CLEAVE;
    if (strcmp(s, "DIVINE_STRIKE_EFFECT") == 0) return DIVINE_STRIKE_EFFECT;
    return None;
}

// to parse stirng taken from catalogue file into a Card type
CardType StringToType(const char* s) {
    if (strcmp(s, "Attack") == 0) return Attack;
    if (strcmp(s, "Heal") == 0) return Heal;
    if (strcmp(s, "Shield") == 0) return Shield;
    return Attack;
}

int LoadCatalogue(const char* fcat, Card* cat_arr, int max_size) {
    // load file
    FILE* catalogue_file = fopen(fcat, "r");
    // if cannot open we read 0 cards so return 0
    if (!catalogue_file) {
        return 0;
    }

    // variables to store data
    char string[255];
    char type[30];
    char effect[30];
    char desc[200];

    // card read count
    int count = 0;

    // get line by line until EOF and card count not max than max number of elenments in the array
    while (fgets(string, 255, catalogue_file) != NULL && count < max_size) {
        // scan the line for the attributes of card struct
        sscanf_s(string, "%29[^,],%29[^,],%d,%198[^\n]", //all takes in one less character to accomodate null termination
            type, (unsigned int)sizeof(type),
            effect, (unsigned int)sizeof(effect),
            &cat_arr[count].power,
            desc, (unsigned int)sizeof(desc));

        // force desc to be null terminated
        desc[199] = '\0';
        
        // set the attribute to a element in the array of type Card
        cat_arr[count].type = StringToType(type);
        cat_arr[count].effect = StringToEffect(effect);
        strcpy_s(cat_arr[count].description, sizeof(cat_arr[count].description), desc);
        cat_arr[count].pos = cat_arr[count].target_pos = CP_Vector_Set(0.0f, 0.0f);
        cat_arr[count].is_animating = false;
        cat_arr[count].is_discarding = false;
        cat_arr[count].card_h = CARD_H_INIT;
        cat_arr[count].card_w = CARD_W_INIT;
        // increment the card
        count++;
    }

    // close the file
    fclose(catalogue_file);
    // return number of card read
    return count;
}