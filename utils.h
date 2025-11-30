// Utility helper functions for collision and interaction.
#pragma once

// Checks if a mouse click at (click_x, click_y) occurred within a rectangular area
// defined by its center (area_center_x, area_center_y) and dimensions (area_width, area_height).
// Returns 1 if clicked inside, 0 otherwise.
int IsAreaClicked(float area_center_x, float area_center_y, float area_width, float area_height, float click_x, float click_y);