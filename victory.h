// Handles the Victory screen displayed after beating the final level.
#pragma once

// Initializes the Victory state, resets timers, and loads fonts.
void Victory_Init(void);

// Updates the Victory state, drawing the success message, score (stars), and restart button.
void Victory_Update(void);

// Cleans up Victory state resources.
void Victory_Exit(void);