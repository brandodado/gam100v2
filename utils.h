#pragma once
#include <stdbool.h>

// --- REMOVED: AreCirclesIntersecting function (it was not defined) ---

// <-- FIX: Added IsAreaClicked declaration here -->
int IsAreaClicked(float area_center_x, float area_center_y,
    float area_width, float area_height,
    float click_x, float click_y);