#pragma once
#include "cprocessing.h"

// Enumeration for sound types to avoid magic strings
typedef enum {
    SFX_CARD_DRAW,
    SFX_HEAL,
    SFX_SHIELD
} SoundType;

void Audio_Init(void);
void Audio_Play(SoundType type);
void Audio_Exit(void);