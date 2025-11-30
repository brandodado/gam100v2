// Audio management system for handling sound effects.
#pragma once
#include "cprocessing.h"

// Enumeration for sound types to avoid magic strings and ensure type safety.
typedef enum {
    SFX_CARD_DRAW,
    SFX_HEAL,
    SFX_SHIELD
} SoundType;

// Initializes the audio system and loads all sound assets.
void Audio_Init(void);

// Plays a specific sound effect based on the provided SoundType enum.
void Audio_Play(SoundType type);

// Cleans up audio resources and frees memory.
void Audio_Exit(void);