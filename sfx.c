#include "sfx.h"
#include <stdio.h>

// Static variables to hold the sound data
static CP_Sound sfx_card_draw = NULL;
static CP_Sound sfx_heal = NULL;
static CP_Sound sfx_shield = NULL;

void Audio_Init(void) {
    // Load sounds once at startup
    // Ensure filenames match exactly what you uploaded
    sfx_card_draw = CP_Sound_Load("Assets/card_in.ogg");
    sfx_heal = CP_Sound_Load("Assets/heal_sfx.ogg");
    sfx_shield = CP_Sound_Load("Assets/shield_sfx.ogg");

    // Error checking (optional but good for debugging)
    if (!sfx_card_draw) printf("Warning: card_in.ogg not found\n");
    if (!sfx_heal)      printf("Warning: heal_sfx.ogg not found\n");
    if (!sfx_shield)    printf("Warning: shield_sfx.ogg not found\n");
}

void Audio_Play(SoundType type) {
    switch (type) {
    case SFX_CARD_DRAW:
        if (sfx_card_draw) CP_Sound_Play(sfx_card_draw);
        break;
    case SFX_HEAL:
        if (sfx_heal) CP_Sound_Play(sfx_heal);
        break;
    case SFX_SHIELD:
        if (sfx_shield) CP_Sound_Play(sfx_shield);
        break;
    }
}

void Audio_Exit(void) {
    // Clean up memory
    CP_Sound_Free(sfx_card_draw);
    CP_Sound_Free(sfx_heal);
    CP_Sound_Free(sfx_shield);
}