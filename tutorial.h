// Handles the multi-page Tutorial system explaining game mechanics.
#pragma once

// Initializes the Tutorial state, loading fonts and resetting to page 1.
void Tutorial_Init(void);

// Updates the Tutorial state, handling page navigation and rendering instructional graphics.
void Tutorial_Update(void);

// Cleans up Tutorial resources.
void Tutorial_Exit(void);