// Handles the Game Over screen displayed when the player dies.
#pragma once

// Initializes the Game Over state, loading fonts and resetting timers.
void GameOver_Init(void);

// Updates the Game Over state, drawing buttons for restart or menu exit.
void GameOver_Update(void);

// Cleans up Game Over state resources.
void GameOver_Exit(void);