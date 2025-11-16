#include "utils.h" // <-- Be sure to include the .h file
#include <math.h>

int IsAreaClicked(float area_center_x, float area_center_y, float area_width, float area_height, float click_x, float click_y)
{
	float left_border = area_center_x - (area_width / 2);
	float right_border = area_center_x + (area_width / 2);
	float top_border = area_center_y - (area_height / 2);
	float bottom_border = area_center_y + (area_height / 2);
	if (left_border <= click_x && right_border >= click_x && top_border <= click_y && bottom_border >= click_y)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

// --- ADD THIS FUNCTION DEFINITION ---
// This placeholder function fixes the "definition not found" error.
// The compiler was looking for this and couldn't find it.
// All your ')' errors were likely caused by a typo in or around this function.
int AreCirclesIntersecting(float x1, float y1, float r1, float x2, float y2, float r2)
{
	// TODO: Write the real collision logic here.

	// For now, just calculate the distance between the two centers
	float dx = x2 - x1;
	float dy = y2 - y1;
	float distance = sqrt(dx * dx + dy * dy);

	// Check if the distance is less than the sum of the two radii
	if (distance < (r1 + r2))
	{
		return 1; // They are intersecting
	}

	return 0; // They are not intersecting
}
